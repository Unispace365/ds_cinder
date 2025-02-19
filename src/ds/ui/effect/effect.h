#ifndef DS_UI_EFFECT_EFFECT_H_
#define DS_UI_EFFECT_EFFECT_H_

#include "ds/ui/sprite/sprite.h"

namespace ds { namespace ui {

	class EffectBlur;

	class EffectType {
	  public:
		virtual ~EffectType() = default;

		virtual void setEffectParams(std::string_view params) = 0;

		virtual void applyEffect(const ci::gl::TextureRef& texture, const ci::ColorA& borderColor) const = 0;
	};

	// *Experimental* Do not use in production!
	class Effect : public Sprite {
	  public:
		Effect(SpriteEngine& engine);

		void setType(const std::string& type);

		void drawClient(const ci::mat4& transformMatrix, const DrawParams& drawParams) override;

	  protected:
		//void onChildAdded(Sprite& child) override {
		//	if (mParent) {
		//		mParent->onChildAdded(child);
		//		mCallbacks[&child] = child.getDimensionsChangedCallback();

		//		// Piggyback the callback.
		//		child.setDimensionsChangedCallback([&](Sprite* sprite) {
		//			if (mCallbacks.count(sprite)) mCallbacks[sprite](sprite);
		//			handleResize();
		//		});
		//	}
		//}

		//void onChildRemoved(Sprite& child) override {
		//	if (mCallbacks.count(&child)) {
		//		child.setDimensionsChangedCallback(mCallbacks[&child]);
		//		mCallbacks.erase(&child);
		//	}

		//	if (mParent) mParent->onChildRemoved(child);

		//	handleResize();
		//}

		void onParentSet() override {
			//// Allow parent to set callbacks on our children, because we're just a wrapper.
			//for (auto child : mChildren) {
			//	onChildAdded(*child);
			//}
			handleResize();
		}

		void handleResize();

	  private:
		std::unique_ptr<EffectType>									  mEffect;
		std::unordered_map<std::string, std::function<EffectType*()>> mEffectTypes;
		std::unordered_map<Sprite*, std::function<void(Sprite*)>>	  mCallbacks;
	};

	class EffectBlur : public EffectType {
	  public:
		EffectBlur()
		  : EffectBlur(9) {}

		/// Creates a Gaussian blur with a specific standard deviation \a sigma. The optional kernel size is rounded up
		/// to the nearest odd integer if greater than 0, otherwise it is calculated based on the standard deviation.
		///	Note: if you find that performance is an issue, you can choose to manually set the kernel size to a lower
		/// value.
		EffectBlur(double sigma, int kernelSize = 0);

		void setEffectParams(std::string_view params) override {}

		void applyEffect(const ci::gl::TextureRef& texture,
						 const ci::ColorA&		   borderColor = ci::ColorA::black()) const override;

		/// Returns the kernel size, which is the number of samples used to calculate the blur.
		///	Note that this implementation only uses half the number of samples, thanks to smart use of bi-linear
		/// filtering
		/// on the GPU.
		int getKernelSize() const { return mKernelSize; }
		/// Returns the standard deviation of the Gaussian distribution.
		double getSigma() const { return mSigma; }

	  private:
		static const char* sVertShader;
		static const char* sFragShader;

		template <typename T>
		T errorFunction(T x) {
			constexpr auto a1 = T(0.254829592);
			constexpr auto a2 = T(-0.284496736);
			constexpr auto a3 = T(1.421413741);
			constexpr auto a4 = T(-1.453152027);
			constexpr auto a5 = T(1.061405429);
			constexpr auto p  = T(0.3275911);

			auto t = T(1) / (T(1) + p * glm::abs(x));
			auto y = T(1) - ((((a5 * t + a4) * t + a3) * t + a2) * t + a1) * t * glm::exp(-x * x);

			auto sign = (x < 0) ? T(-1) : T(1);
			return sign * y;
		}

		template <typename T>
		T gaussianDistribution(T x, T mu, T sigma) {
			static const auto sqrtOfTwo = glm::sqrt(T(2));
			return T(0.5) * errorFunction((x - mu) / (sqrtOfTwo * sigma));
		}

		double						mThreshold	= 0; // Weights below this threshold are not used by the shader.
		double						mSigma		= 1; // Gaussian distribution factor.
		int							mKernelSize = 5; // Equivalent Gaussian kernel size (e.g. 5x5).
		std::vector<double>			mWeights;		 // List of calculated weight for each sample.
		std::vector<double>			mOffsets;		 // List of calculated offset for each sample.
		mutable ci::gl::FboRef		mFbo[2];		 //
		mutable ci::gl::GlslProgRef mGlsl;			 //
	};

}} // namespace ds::ui

#endif