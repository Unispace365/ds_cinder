#include "stdafx.h"

#include "ds/rive/nvpath/rive_paint_nvpath.h"
#include "ds/rive/nvpath/rive_renderer_nvpath.h"

namespace ds {

void RiveRendererNvPath::save() {
	mTransformStack.push_back(mTransformStack.back());
	mClipCountStack.push_back(mClipCountStack.back());
}

void RiveRendererNvPath::restore() {
	mTransformStack.pop_back();

	size_t clipCount = mClipCountStack.back();
	mClipCountStack.pop_back();
	while (clipCount-- > mClipCountStack.back())
		nvpath::popClipPath();
}

void RiveRendererNvPath::transform(const rive::Mat2D& transform) {
	mTransformStack.back() =
		mTransformStack.back() * glm::mat4(transform[0], transform[1], 0, 0, transform[2], transform[3], 0, 0, 0, 0, 1,
										   0, transform[4], transform[5], 0, 1);
}

void RiveRendererNvPath::drawPath(rive::RenderPath* path, rive::RenderPaint* paint) {
	auto thePath  = dynamic_cast<RivePathNvPath*>(path);
	auto thePaint = dynamic_cast<RivePaintNvPath*>(paint);
	if (thePath && thePaint) {
		ci::gl::ScopedModelMatrix sm;
		ci::gl::multModelMatrix(mTransformStack.back());

		thePaint->render(thePath);
	}
}

void RiveRendererNvPath::clipPath(rive::RenderPath* path) {
	auto thePath = dynamic_cast<RivePathNvPath*>(path);
	if (thePath) {
		nvpath::pushClipPath(thePath->getPath());
		++mClipCountStack.back();
	}
}

void RiveRendererNvPath::drawImage(const rive::RenderImage*, rive::BlendMode, float opacity) {}

void RiveRendererNvPath::drawImageMesh(const rive::RenderImage*, rive::rcp<rive::RenderBuffer> vertices_f32,
									   rive::rcp<rive::RenderBuffer> uvCoords_f32,
									   rive::rcp<rive::RenderBuffer> indices_u16, uint32_t vertexCount,
									   uint32_t indexCount, rive::BlendMode, float opacity) {}

} // namespace ds