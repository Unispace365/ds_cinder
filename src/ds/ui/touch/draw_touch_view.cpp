#include "stdafx.h"

#include "draw_touch_view.h"

#include <cinder/TriMesh.h>
#include <cinder/gl/Vbo.h>

#include "ds/ui/sprite/circle.h"

namespace {

const std::string VertShader =
	//"#version 150\n"
	"uniform mat4       ciModelViewProjection;\n"
	"in vec4            ciPosition;\n"
	"in vec4            ciColor;\n"
	"in vec4            Circle;\n" // xy = position, z = reserved, w = id
	"out vec4           Color;\n"
	"void main()\n"
	"{\n"
	"   Color = ciColor;\n"
	"	gl_Position = ciModelViewProjection * vec4(ciPosition.xy + Circle.xy, 0.0, 1.0);\n"
	"}\n";

const std::string FragShader =
	//"#version 150\n"
	"in vec4            Color;\n"
	"out vec4           oColor;\n"
	"void main()\n"
	"{\n"
	"    oColor = Color;\n"
	"}\n";

} // namespace

namespace ds { namespace ui {

	DrawTouchView::DrawTouchView(SpriteEngine& e)
	  : Sprite(e)
	  , mCircleRadius(20.0f)
	  , mCircleFilled(false)
	  , mCircleColor(ci::ColorA::white()) {}

	DrawTouchView::DrawTouchView(SpriteEngine& e, cfg::Settings& settings, TouchManager& tm)
	  : Sprite(e)
	  , mCircleRadius(20.0f)
	  , mCircleFilled(false)
	  , mCircleColor(ci::ColorA::white()) {
		setTransparent(false);

		tm.setCapture(this);

		mCircleRadius = settings.getFloat("touch:debug_circle_radius", 0, mCircleRadius);
		mCircleColor  = settings.getColorA(dynamic_cast<Engine&>(e), "touch:debug_circle_color", 0, mCircleColor);
		mCircleFilled = settings.getBool("touch:debug_circle_filled", 0, mCircleFilled);

		mSpriteShader = SpriteShader(VertShader, FragShader, "ds::ui::DrawTouchView");

		mInstances.reserve(256);
	}

	void DrawTouchView::onBuildRenderBatch() {
		if (mRenderBatch) return;
		if (mCircleRadius <= 0.0f) return;

		ci::gl::VboMeshRef theMesh;
		if (mCircleFilled) {
			auto theCircle = DsCircleGeom().radius(mCircleRadius).center(ci::vec2(0, 0));
			theMesh		   = ci::gl::VboMesh::create(theCircle);
		} else {
			auto theCircle = ci::geom::Ring().radius(mCircleRadius - 0.5f).width(1.0f).center(ci::vec2(0, 0));
			theMesh		   = ci::gl::VboMesh::create(theCircle);
		}

		mBuffer =
			ci::gl::Vbo::create(GL_ARRAY_BUFFER, mInstances.capacity() * sizeof(Instance), nullptr, GL_STATIC_DRAW);

		constexpr auto			   stride = sizeof(Instance);
		cinder::geom::BufferLayout instanceDataLayout;
		instanceDataLayout.append(cinder::geom::Attrib::CUSTOM_0, 4, stride, offsetof(Instance, position),
								  1 /* per instance */);

		ci::gl::Batch::AttributeMapping mapping;
		mapping[cinder::geom::Attrib::CUSTOM_0] = "Circle";

		theMesh->appendVbo(instanceDataLayout, mBuffer);

		mRenderBatch = ci::gl::Batch::create(theMesh, mSpriteShader.getShader(), mapping);
	}

	void DrawTouchView::touchBegin(const TouchInfo& ti) {
		auto itr = std::find(mInstances.begin(), mInstances.end(), ti.mFingerId);
		if (itr != mInstances.end()) {
			itr->position = ti.mCurrentGlobalPoint;
		} else {
			size_t capacity = mInstances.capacity();
			mInstances.push_back(Instance{ti.mCurrentGlobalPoint, mCircleRadius, ti.mFingerId});
			if (mInstances.capacity() > capacity) {
				mRenderBatch.reset();
			}
		}
	}

	void DrawTouchView::touchMoved(const TouchInfo& ti) {
		auto itr = std::find(mInstances.begin(), mInstances.end(), ti.mFingerId);
		if (itr != mInstances.end()) {
			itr->position = ti.mCurrentGlobalPoint;
		}
	}

	void DrawTouchView::touchEnd(const TouchInfo& ti) {
		auto itr = std::find(mInstances.begin(), mInstances.end(), ti.mFingerId);
		if (itr != mInstances.end()) {
			mInstances.erase(itr);
		}
	}

	void DrawTouchView::drawLocalClient() {
		if (mInstances.empty() || !mRenderBatch || !mBuffer) return;

		auto* ptr = static_cast<Instance*>(mBuffer->mapReplace());
		for (const auto& inst : mInstances) {
			*ptr++ = inst;
		}
		mBuffer->unmap();

		ci::gl::ScopedColor sc(mCircleColor);
		mRenderBatch->drawInstanced(static_cast<GLsizei>(mInstances.size()));
	}

}} // namespace ds::ui
