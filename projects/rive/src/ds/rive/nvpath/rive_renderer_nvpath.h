#pragma once

#include <rive/refcnt.hpp>
#include <rive/renderer.hpp>

namespace ds {

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

} // namespace ds