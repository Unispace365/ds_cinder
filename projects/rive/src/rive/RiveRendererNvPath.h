#pragma once

#include <iomanip>
#include <sstream>
#include <string>

#include <nvpath/NvPath.h>

#include "rive/factory.hpp"
#include "rive/refcnt.hpp"
#include "rive/renderer.hpp"

namespace ds { namespace ui {

	class RivePath : public rive::RenderPath {
	  public:
		RivePath() = default;
		RivePath(const rive::RawPath& path, rive::FillRule fillRule);

		void addRenderPath(RenderPath* path, const rive::Mat2D& transform) override;
		void addRawPath(const rive::RawPath& path) override;

		void rewind() override;
		void fillRule(rive::FillRule value) override;
		void addPath(CommandPath* path, const rive::Mat2D& transform) override;

		void moveTo(float x, float y) override;
		void lineTo(float x, float y) override;
		void cubicTo(float ox, float oy, float ix, float iy, float x, float y) override;
		void close() override;

		RenderPath*		  renderPath() override;
		const RenderPath* renderPath() const override;

		nvpath::Path& getPath() const { return mPath; }

	  private:
		mutable nvpath::Path mPath;
		nvpath::PathHelper	 mHelper;
	};

	class RivePaint : public rive::RenderPaint {
	  public:
		RivePaint() = default;

		void style(rive::RenderPaintStyle style) override;
		void color(rive::ColorInt value) override;
		void thickness(float value) override;
		void join(rive::StrokeJoin value) override;
		void cap(rive::StrokeCap value) override;
		void feather(float value) override {} // Not supported.
		void blendMode(rive::BlendMode value) override;
		void shader(rive::rcp<rive::RenderShader>) override;
		void invalidateStroke() override {}

		void render(const RivePath* path) const;

	  private:
		nvpath::Paint	  mPaint;
		nvpath::CapsStyle mCapsStyle;
		nvpath::JoinStyle mJoinStyle;
		float			  mStrokeWidth{1.0f};
		bool			  mIsStroke{false};
	};

	class RiveGradient : public rive::RenderShader {
	  public:
		RiveGradient() = default;

		const nvpath::Paint& getPaint() const { return mPaint; }

		static std::string pointerToUid(const void* ptr) {
			std::ostringstream oss;
			oss << std::hex << std::setw(sizeof(ptr) * 2) << std::setfill('0') << reinterpret_cast<uintptr_t>(ptr);
			return oss.str();
		}

		static rive::rcp<RiveGradient> makeLinearGradient(float sx, float sy, float ex, float ey,
														  const rive::ColorInt colors[], const float stops[],
														  size_t count);

		static rive::rcp<RiveGradient> makeRadialGradient(float cx, float cy, float radius,
														  const rive::ColorInt colors[], const float stops[],
														  size_t count);

	  private:
		nvpath::Paint mPaint;
	};

	class RiveFactoryNvPath : public rive::Factory {
	  public:
		rive::rcp<rive::RenderBuffer> makeRenderBuffer(rive::RenderBufferType, rive::RenderBufferFlags,
													   size_t sizeInBytes) override;

		rive::rcp<rive::RenderShader> makeLinearGradient(float sx, float sy, float ex, float ey,
														 const rive::ColorInt colors[], // [count]
														 const float		  stops[],	// [count]
														 size_t				  count) override;

		rive::rcp<rive::RenderShader> makeRadialGradient(float cx, float cy, float radius,
														 const rive::ColorInt colors[], // [count]
														 const float		  stops[],	// [count]
														 size_t				  count) override;


		rive::rcp<rive::RenderPath> makeRenderPath(rive::RawPath&, rive::FillRule) override;

		// Deprecated -- working to make RenderPath's immutable
		rive::rcp<rive::RenderPath> makeEmptyRenderPath() override;

		rive::rcp<rive::RenderPaint> makeRenderPaint() override;

		rive::rcp<rive::RenderImage> decodeImage(rive::Span<const uint8_t>) override { return {}; }
	};

	class RiveRendererNvPath : public rive::Renderer {
	  public:
		RiveRendererNvPath() {
			mTransformStack.emplace_back();
			mClipCountStack.emplace_back(0);
		}

		void save() override;
		void restore() override;
		void transform(const rive::Mat2D& transform) override;
		void drawPath(rive::RenderPath* path, rive::RenderPaint* paint) override;
		void clipPath(rive::RenderPath* path) override;
		void drawImage(const rive::RenderImage*, rive::BlendMode, float opacity) override;
		void drawImageMesh(const rive::RenderImage*, rive::rcp<rive::RenderBuffer> vertices_f32,
						   rive::rcp<rive::RenderBuffer> uvCoords_f32, rive::rcp<rive::RenderBuffer> indices_u16,
						   uint32_t vertexCount, uint32_t indexCount, rive::BlendMode, float opacity) override;

	  private:
		std::vector<glm::mat4> mTransformStack;
		std::vector<size_t>	   mClipCountStack;
	};

}} // namespace ds::ui