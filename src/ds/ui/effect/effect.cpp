#include "stdafx.h"

#include "effect.h"

#include "ds/util/float_util.h"

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
	  , mKernelSize(kernelSize <= 0 ? int(sigma * 2 + 1) : kernelSize + +(kernelSize % 2 == 0)) {
		calculateWeightsAndOffsets();
	}

	void EffectBlur::applyEffect(const ci::gl::TextureRef& texture, const ci::ColorA& borderColor) const {
		applyEffect(texture, texture->getBounds(), borderColor);
	}

	void EffectBlur::applyEffect(const ci::gl::TextureRef& texture, const ci::Area& bounds,
								 const ci::ColorA& borderColor) const {
		// Create intermediate textures if necessary.
		const auto size = bounds.getSize();

		ci::gl::FboRef fbo[2];
		{
			auto textureFormat = ci::gl::Texture::Format().internalFormat(texture->getInternalFormat());
			textureFormat.setBorderColor(borderColor);

			const auto fboFormat = ci::gl::Fbo::Format().colorTexture(textureFormat).disableDepth().samples(0);
			fbo[0]				 = ci::gl::Fbo::create(size.x, size.y, fboFormat);
		}
		{
			ci::gl::ScopedTextureBind st(texture);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor.ptr());

			const auto fboFormat =
				ci::gl::Fbo::Format().attachment(GL_COLOR_ATTACHMENT0, texture).disableDepth().samples(0);
			fbo[1] = ci::gl::Fbo::create(size.x, size.y, fboFormat);
		}

		// Compile shaders if necessary.
		if (!mGlsl) {
			try {
				std::string blur;
				blur += "    fragColor += " + std::to_string(mWeights[0]) + " * texture( uInput, vertTexCoord );\n";
				for (int i = 1; i < static_cast<int>(mWeights.size()); ++i) {
					std::string weight = std::to_string(mWeights[i]);
					std::string offset = std::to_string(mOffsets[i]);
					blur += "    fragColor += " + weight;
					blur += " * texture( uInput, vertTexCoord - " + offset;
					blur += " * uStep );\n";
					blur += "    fragColor += " + weight;
					blur += " * texture( uInput, vertTexCoord + " + offset;
					blur += " * uStep );\n";
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
		if (fbo[0] && fbo[1] && mGlsl) {
			ci::gl::ScopedBlend	   scpBlend(false);
			ci::gl::ScopedColor	   scpColor(1, 1, 1);
			ci::gl::ScopedGlslProg scpGlsl(mGlsl);
			ci::gl::ScopedViewport scopedViewport(texture->getSize());
			// ci::gl::ScopedMatrices scopedMatrices;
			// ci::gl::setMatricesWindow(texture->getSize());

			{
				mGlsl->uniform("uStep", ci::vec2(1.0f / size.x, 0));

				ci::gl::ScopedTextureBind scpInput(texture, 0);
				ci::gl::ScopedFramebuffer scopedFbo(fbo[0]);

				ci::gl::clear(ci::ColorA(0, 0, 0, 0));

				ci::gl::begin(GL_TRIANGLE_STRIP);
				ci::gl::vertex(-1, -1);
				ci::gl::vertex(+1, -1);
				ci::gl::vertex(-1, +1);
				ci::gl::vertex(+1, +1);
				ci::gl::end();
			}

			// Resolve textures if necessary.
			fbo[0]->resolveTextures();

			{
				mGlsl->uniform("uStep", ci::vec2(0, 1.0f / size.y));

				ci::gl::ScopedTextureBind scpInput(fbo[0]->getColorTexture(), 0);
				ci::gl::ScopedFramebuffer scopedFbo(fbo[1]);

				ci::gl::clear(ci::ColorA(0, 0, 0, 0));

				ci::gl::begin(GL_TRIANGLE_STRIP);
				ci::gl::vertex(-1, -1);
				ci::gl::vertex(+1, -1);
				ci::gl::vertex(-1, +1);
				ci::gl::vertex(+1, +1);
				ci::gl::end();
			}

			// Resolve textures if necessary.
			fbo[1]->resolveTextures();
		}
	}

	void EffectBlur::setSigma(double sigma, int kernelSize) {
		mSigma		= sigma;
		mKernelSize = kernelSize <= 0 ? int(sigma * 2 + 1) : kernelSize + (kernelSize % 2 == 0);
		calculateWeightsAndOffsets();
	}

	void EffectBlur::calculateWeightsAndOffsets() {
		// Calculate gaussian kernel.
		std::vector<double> weights;
		std::vector<double> offsets;

		// We only define one half of the distribution.
		const auto sz = mKernelSize / 2 + 1;
		weights.reserve(sz);
		offsets.reserve(sz);

		auto sum = 0.0;
		auto x	 = gaussianDistribution(-0.5, 0.0, mSigma);
		for (int i = 0; i < sz; ++i) {
			offsets.emplace_back(i);

			auto y = gaussianDistribution(static_cast<double>(i) + 0.5, 0.0, mSigma);
			if (y - x < 1.0e-4) break;
			weights.emplace_back(y - x);
			sum += weights.back();

			x = y;
		}

		// Normalize weights.
		sum = 2.0 * sum - weights.front();
		for (auto& weight : weights)
			weight /= sum;

		// Sanity check.
		sum = weights.front();
		for (size_t i = 1; i < weights.size(); ++i)
			sum += 2.0 * weights[i];
		assert(ds::approxEqual(1.0, sum));

		// Now, optimize the kernel by exploiting linear filtering on the GPU.
		// See also: https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
		mWeights.clear();
		mOffsets.clear();

		mWeights.reserve(sz);
		mOffsets.reserve(sz);

		mWeights.emplace_back(weights.front());
		mOffsets.emplace_back(offsets.front());

		sum = mWeights.front();
		for (size_t i = 1; i + 1 < weights.size(); i += 2) {
			double weight = weights[i] + weights[i + 1];
			double offset = (offsets[i] * weights[i] + offsets[i + 1] * weights[i + 1]) / weight;
			mWeights.emplace_back(weight);
			mOffsets.emplace_back(offset);
			sum += 2.0 * mWeights.back();
		}

		// Normalize weights.
		for (auto& weight : mWeights)
			weight /= sum;

		// Sanity check.
		sum = mWeights.front();
		for (size_t i = 1; i < mWeights.size(); ++i)
			sum += 2.0 * mWeights[i];
		assert(ds::approxEqual(1.0, sum));

		// Invalidate shader.
		mGlsl.reset();
	}

}} // namespace ds::ui