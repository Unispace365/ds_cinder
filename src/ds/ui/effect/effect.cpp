#include "stdafx.h"

#include "effect.h"

namespace {

// Add the 'path' sprite type so we can use it in our layout XML.
auto INIT = []() {
	ds::App::AddStartup("Effect", [](ds::Engine& e) {
		using namespace ds::ui;

		// Register our custom sprite(s).
		e.registerSpriteImporter("effect",
								 [](ds::ui::SpriteEngine& engine) -> ds::ui::Sprite* { return new Effect(engine); });

		// Register the properties for our custom sprites.
		e.registerSpritePropertySetter<Effect>(
			"type", [](Effect& path, const std::string& theValue, const std::string&) { path.setType(theValue); });
	});
	return true;
}();

} // namespace

namespace ds { namespace ui {

	Effect::Effect(SpriteEngine& engine)
	  : Sprite(engine) {
		setWrapper(true);
		setFinalRenderToTexture(true);

		// For now:
		mEffect = std::make_unique<EffectBlur>();
	}

	void Effect::setType(const std::string& type) {
		auto itr = mEffectTypes.find(type);
		if (itr != mEffectTypes.end()) {
			mEffect = std::unique_ptr<EffectType>(itr->second());
		}
	}

	void Effect::drawClient(const ci::mat4& transformMatrix, const DrawParams& drawParams) {
		Sprite::drawClient(transformMatrix, drawParams);

		if (mOutputFbo) {
			if (mEffect) mEffect->applyEffect(mOutputFbo->getColorTexture(), ci::ColorA::black());

			ci::gl::color(1, 0, 0);

			const auto bounds = ci::Rectf(0, 0, mWidth / mScale.x, mHeight / mScale.y);
			ci::gl::draw(mOutputFbo->getColorTexture(), bounds);
		}
	}

	void Effect::handleResize() {
		ci::Rectf bounds{0, 0, 0, 0};

		for (auto child : mChildren) {
			ci::vec2 position = child->getPosition();
			ci::vec2 size	  = child->getSize();
			bounds.include(ci::Rectf(position.x, position.y, position.x + size.x, position.y + size.y));
		}

		constexpr float factor = 1.0f;
		setPosition(bounds.x1, bounds.y1);
		setSize(bounds.getWidth() * factor, bounds.getHeight() * factor);
		setScale(factor);
	}

	const char* EffectBlur::sVertShader = //
		"#version 150\n"
		"in vec4 ciPosition;"	 // In range [-1...1].
		"out vec2 vertTexCoord;" // In range [0...1].
		"void main(void) {"
		"    vertTexCoord = ciPosition.xy * 0.5 + 0.5;"
		"    gl_Position = ciPosition;"
		"}";

	const char* EffectBlur::sFragShader = //
		"#version 150\n"
		"uniform sampler2D uInput;\n"
		"uniform vec2      uStep;\n" // (1, 0) for horizontal pass, (0, 1) for vertical pass.
		"in  vec2 vertTexCoord;\n"	 // In range [0...1].
		"out vec4 fragColor;\n"
		"void main(void) {\n"
		"    fragColor = vec4( 0, 0, 0, 0 );\n"
		"///***///"
		"}";

	EffectBlur::EffectBlur(double sigma, int kernelSize)
	  : mSigma(sigma)
	  , mKernelSize(kernelSize > 0 ? kernelSize + (kernelSize % 2 == 0) : static_cast<int>(2 * floor(sigma * 2) + 1)) {
		// Calculate gaussian kernel.
		std::vector<double> weights;
		std::vector<double> offsets;

		const int sz = (mKernelSize - 1) / 2 + 1; // We only define one half of the distribution.
		weights.reserve(static_cast<size_t>(sz));
		offsets.reserve(static_cast<size_t>(sz));

		auto sum = 0.0;
		auto x	 = gaussianDistribution(-0.5, 0.0, sigma);
		for (int i = 0; i < sz; ++i) {
			auto y = gaussianDistribution(static_cast<double>(i) + 0.5, 0.0, sigma);
			weights.emplace_back(y - x);
			offsets.emplace_back(i);
			std::swap(x, y);

			sum += weights.back();
		}

		// Normalize weights. Only count the center once, but double the rest, since we've only defined half the
		// distribution.
		sum = 2.0 * sum - weights.front();
		for (auto& weight : weights)
			weight /= sum;

		// Now, optimize the kernel by exploiting linear filtering on the GPU.
		// See also: https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
		mWeights.reserve(static_cast<size_t>(sz / 2) + 1);
		mOffsets.reserve(static_cast<size_t>(sz / 2) + 1);

		mWeights.emplace_back(weights.front());
		mOffsets.emplace_back(offsets.front());

		for (int i = 0; i < (sz - 1) / 2; ++i) {
			const size_t j		= static_cast<size_t>(i) * 2 + 1;
			double		 weight = weights[j] + weights[j + 1];
			double		 offset = (offsets[j] * weights[j] + offsets[j + 1] * weights[j + 1]) / weight;
			mWeights.emplace_back(weight);
			mOffsets.emplace_back(offset);
		}
	}

	void EffectBlur::applyEffect(const ci::gl::TextureRef& texture, const ci::ColorA& borderColor) const {
		// Create intermediate textures if necessary.
		const auto size = texture->getSize();

		{
			auto textureFormat = ci::gl::Texture::Format().internalFormat(texture->getInternalFormat());
			textureFormat.setBorderColor(borderColor);

			const auto fboFormat = ci::gl::Fbo::Format().colorTexture(textureFormat).disableDepth().samples(0);
			mFbo[0]				 = ci::gl::Fbo::create(size.x, size.y, fboFormat);
		}
		{
			ci::gl::ScopedTextureBind st(texture);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor.ptr());

			const auto fboFormat =
				ci::gl::Fbo::Format().attachment(GL_COLOR_ATTACHMENT0, texture).disableDepth().samples(0);
			mFbo[1] = ci::gl::Fbo::create(size.x, size.y, fboFormat);
		}

		// Compile shaders if necessary.
		if (!mGlsl) {
			try {
				std::string blur;
				blur += "    fragColor += " + std::to_string(mWeights[0]) + " * texture( uInput, vertTexCoord );\n";
				for (int i = 1; i < static_cast<int>(mWeights.size()); ++i) {
					if (mWeights[i] > mThreshold) {
						blur += "    fragColor += " + std::to_string(mWeights[i]) +
								" * texture( uInput, vertTexCoord - " + std::to_string(mOffsets[i]) + " * uStep );\n";
						blur += "    fragColor += " + std::to_string(mWeights[i]) +
								" * texture( uInput, vertTexCoord + " + std::to_string(mOffsets[i]) + " * uStep );\n";
					}
				}

				std::string token	   = "///***///";
				std::string fragShader = sFragShader;
				fragShader.replace(fragShader.find(token), token.length(), blur);

				mGlsl = ci::gl::GlslProg::create(sVertShader, fragShader);
				mGlsl->uniform("uInput", 0);
			} catch (const std::exception& exc) {
				ci::app::console() << exc.what() << std::endl;
			}
		}

		// Apply effect.
		if (mFbo[0] && mFbo[1] && mGlsl) {
			ci::gl::ScopedBlend	   scpBlend(false);
			ci::gl::ScopedColor	   scpColor(1, 1, 1);
			ci::gl::ScopedGlslProg scpGlsl(mGlsl);

			if (mFbo[0]) {
				mGlsl->uniform("uStep", ci::vec2(1, 0) / ci::vec2(size));

				ci::gl::ScopedTextureBind scpInput(texture, 0);
				ci::gl::ScopedFramebuffer scopedFbo(mFbo[0]);
				ci::gl::ScopedViewport	  scopedViewport(mFbo[0]->getSize());

				ci::gl::clear(ci::ColorA(0, 0, 0, 0));

				ci::gl::begin(GL_TRIANGLE_STRIP);
				ci::gl::vertex(-1, -1);
				ci::gl::vertex(+1, -1);
				ci::gl::vertex(-1, +1);
				ci::gl::vertex(+1, +1);
				ci::gl::end();

				// Resolve textures if necessary.
				mFbo[0]->resolveTextures();
			}

			if (mFbo[1]) {
				mGlsl->uniform("uStep", ci::vec2(0, 1) / ci::vec2(size));

				ci::gl::ScopedTextureBind scpInput(mFbo[0]->getColorTexture(), 0);
				ci::gl::ScopedFramebuffer scopedFbo(mFbo[1]);
				ci::gl::ScopedViewport	  scopedViewport(mFbo[1]->getSize());

				ci::gl::clear(ci::ColorA(0, 0, 0, 0));

				ci::gl::begin(GL_TRIANGLE_STRIP);
				ci::gl::vertex(-1, -1);
				ci::gl::vertex(+1, -1);
				ci::gl::vertex(-1, +1);
				ci::gl::vertex(+1, +1);
				ci::gl::end();

				// Resolve textures if necessary.
				mFbo[1]->resolveTextures();
			}
		}
	}

}} // namespace ds::ui