#include "stdafx.h"

#include "dashed_line.h"

#include <ds/ui/sprite/sprite_engine.h>


namespace ds { namespace ui {

	const std::string dashedLineFrag =
		//"#version 150\n"
		"in vec4            theColor;\n"
		"out vec4           oColor;\n"
		"void main()\n"
		"{\n"
		"   oColor = theColor;\n"
		"}\n";

	const std::string dashedLineVert =
		"#version 150\n"
		"uniform mat4       ciModelMatrix;\n"
		"uniform mat4       ciModelViewProjection;\n"
		"in vec3            vInstancePosition;\n"
		"in vec3            vInstanceScale;\n"
		"in vec4            ciPosition;\n"
		"in vec4            ciColor;\n"
		"out vec4           theColor;\n"
		"void main()\n"
		"{\n"
		"    vec4 adjPos = vec4(ciPosition.x, ciPosition.y, 1, 1);\n"
		"    adjPos = adjPos * vec4(vInstanceScale.xy * vInstanceScale.z, 1, 1);\n"
		"    gl_Position = ciModelViewProjection * (adjPos + vec4( vInstancePosition, 0 ) );\n"
		"\n"
		"    theColor = ciColor; \n"
		"}\n";

	DashedLine::DashedLine(ds::ui::SpriteEngine& eng, const float lineWidth, const float lineLength,
						   const float dashInc, const float spaceInc)
	  : ds::ui::Sprite(eng, lineWidth, lineLength)
	  , mDashLength(dashInc)
	  , mSpaceIncrement(spaceInc)
	  , mBatchRef(nullptr)
	  , mTotalDashesSize(0) {

		setTransparent(false);

		rebuildLine();
	}

	void DashedLine::setDashLength(const float dashLength) {
		mDashLength = dashLength;
		rebuildLine();
	}

	void DashedLine::setSpaceIncrement(const float spaceIncrement) {
		mSpaceIncrement = spaceIncrement;
		rebuildLine();
	}

	void DashedLine::onSizeChanged() {
		rebuildLine();
	}

	void DashedLine::rebuildLine() {
		// generate line batch
		std::vector<ci::gl::VboMesh::Layout> bufferLayout = {
			ci::gl::VboMesh::Layout().usage(GL_STATIC_DRAW).attrib(ci::geom::Attrib::POSITION, 3)};

		// if sprite is wider than it is tall, prep for horizontal dashed line; otherwise, prep for vertical dashed line
		const bool isHoriz	= getWidth() > getHeight();
		auto	   length	= isHoriz ? getWidth() : getHeight();
		auto	   totalInc = mDashLength + mSpaceIncrement;

		mTotalDashesSize = totalInc > 0.f ? static_cast<int>(length / totalInc) : 0;

		ci::gl::VboMeshRef mesh;

		const auto xThickness = isHoriz ? mDashLength : getWidth();
		const auto yThickness = isHoriz ? getHeight() : mDashLength;
		mesh				  = ci::gl::VboMesh::create(ci::geom::Rect(ci::Rectf(0.f, 0.f, xThickness, yThickness)));

		try {
			mGlsl = ci::gl::GlslProg::create(dashedLineVert, dashedLineFrag);
		} catch (std::exception& e) {
			DS_LOG_WARNING("Dashed Line glsl compile error: " << e.what());
			return;
		}

		std::vector<ci::vec3> positions;
		std::vector<ci::vec3> scales;

		auto pos = ci::vec3(0.0f);
		auto inc = isHoriz ? ci::vec3(totalInc, 0.f, 0.f) : ci::vec3(0.f, totalInc, 0.f);
		for (size_t index = 0; index < mTotalDashesSize; ++index) {
			positions.push_back(pos);
			scales.push_back(ci::vec3(1.f));
			pos += inc;
		}

		// check to see if space for a partial dash
		auto spaceLeft = length - totalInc * mTotalDashesSize;
		if (spaceLeft > 0.f) {
			positions.push_back(pos);
			auto percentLeft  = spaceLeft / mDashLength;
			auto partialScale = isHoriz ? ci::vec3(percentLeft, 1.f, 1.f) : ci::vec3(1.f, percentLeft, 1.f);
			scales.push_back(partialScale);
			mTotalDashesSize++;
		}

		// create the VBO which will contain per-instance (rather than per-vertex) data
		auto posVbo = ci::gl::Vbo::create(GL_ARRAY_BUFFER, positions.size() * sizeof(ci::vec3), positions.data(),
										  GL_DYNAMIC_DRAW);
		auto scaleVbo =
			ci::gl::Vbo::create(GL_ARRAY_BUFFER, scales.size() * sizeof(ci::vec3), scales.data(), GL_DYNAMIC_DRAW);

		// we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather
		// than 0) as the last param indicates per-instance (rather than per-vertex)
		ci::geom::BufferLayout positionLayout;
		positionLayout.append(ci::geom::Attrib::CUSTOM_0, 3, sizeof(ci::vec3), 0, 1 /* per instance */);
		ci::geom::BufferLayout scaleLayout;
		scaleLayout.append(ci::geom::Attrib::CUSTOM_2, 3, sizeof(ci::vec3), 0, 1 /* per instance */);

		// now append the per-instance Vbo's to the VboMesh
		mesh->appendVbo(positionLayout, posVbo);
		mesh->appendVbo(scaleLayout, scaleVbo);

		ci::gl::Batch::AttributeMapping attriMap;
		attriMap[ci::geom::Attrib::CUSTOM_0] = "vInstancePosition";
		attriMap[ci::geom::Attrib::CUSTOM_2] = "vInstanceScale";

		mBatchRef = ci::gl::Batch::create(mesh, mGlsl, attriMap);
	}

	void DashedLine::drawLocalServer() {
		drawLocalClient();
	}

	void DashedLine::drawLocalClient() {
		if (mBatchRef) mBatchRef->drawInstanced((GLsizei)mTotalDashesSize);
	}

}} // namespace ds::ui
