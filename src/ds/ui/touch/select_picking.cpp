#include "select_picking.h"

#include <cinder/gl/gl.h>
#include "ds/app/engine/engine_roots.h"
#include "ds/params/draw_params.h"
#include "ds/ui/sprite/sprite_engine.h"

namespace ds {

/**
 * \class ds::SelectPicking
 * Useful reference:
 * http://web.engr.oregonstate.edu/~mjb/cs553/Handouts/Picking/picking.pdf
 */
SelectPicking::SelectPicking() {
}

static void gluPickMatrix(double x, double y, double deltax, double deltay, int* viewport) {
	if (deltax <= 0 || deltay <= 0) {
		return;
	}

	ci::gl::translate(	static_cast<float>((viewport[2] - 2 * (x - viewport[0])) / deltax),
						static_cast<float>((viewport[3] - 2 * (y - viewport[1])) / deltay),
						0.0f);
	ci::gl::scale(		static_cast<float>(viewport[2] / deltax),
						static_cast<float>(viewport[3] / deltay),
						1.0);
}

ds::ui::Sprite* SelectPicking::pickAt(const ci::Vec2f& pt, ds::ui::Sprite& root) {
	mHits.clear();

	glPushAttrib( GL_VIEWPORT_BIT );
//	const float		picking_offset = 0.0f;
//	glViewport( 0, 0, static_cast<int>(mWorldSize.x + picking_offset), static_cast<int>(mWorldSize.y));
	GLint			viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	ci::Matrix44f	m = ci::gl::getProjection();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix(pt.x, mWorldSize.y-pt.y, 8, 8, viewport);
	ci::gl::multProjection(m);

	// Scale Model-View matrix to simulator scale
//	glMatrixMode( GL_MODELVIEW );

#if 0
	glPushAttrib( GL_VIEWPORT_BIT );
	glViewport( 0, 0, masterWidth + mPickingOffset, masterHeight );
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );
	if(mPickingOffset != 0)	viewport[2] = masterWidth;
	// Set Projection matrix to frustrum around the pick-point
	setupSimulatorView();

	GLfloat mat[16];
	glGetFloatv( GL_PROJECTION_MATRIX, mat );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPickMatrix( x, masterHeight-y, 8, 8, viewport );
	glMultMatrixf( mat );

	// Scale Model-View matrix to simulator scale
	glMatrixMode( GL_MODELVIEW );
#endif

	// Set up picking buffer
	glSelectBuffer(SELECT_BUFFER_SIZE, mSelectBuffer);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	ds::DrawParams			dp;
	dp.mParentOpacity = 1.0f;
	root.drawServer(ci::gl::getModelView(), dp);

	glPopAttrib();

	// Process Hits
	const int				num_hits = glRenderMode(GL_RENDER);
	GLuint*					ptr = mSelectBuffer;
	for (int i=0; i<num_hits; i++) {
		int numNamesInHit = *(ptr+0);
		// There might be multiple names in this hit, but the framework
		// only puts one on the stack, so just take the first.
		if (numNamesInHit > 0) {
			unsigned min_z = *(ptr+1);
//			unsigned max_z = *(ptr+2);
			int id = *(ptr+3);
			mHits.push_back(Hit(id, min_z));
		}

		ptr += 3 + numNamesInHit;
	}

	if (mHits.empty()) return nullptr;

	// The z value is a GL-internal representation. The item with the lowest
	// z is closet to the user.
	std::sort(mHits.begin(), mHits.end(), [](const Hit& a, const Hit& b)->bool { return a > b; });
	// Note -- taking the back. The docs say the low z is the closest, but
	// it seems like the reverse is true, at least in the current app using this.
//	return root.getEngine().findSprite(mHits.front().mId);
	return root.getEngine().findSprite(mHits.back().mId);
}

/**
 * \class ds::SelectPicking::Hit
 */
SelectPicking::Hit::Hit()
		: mId(-1)
		, mZ(0) {
}

SelectPicking::Hit::Hit(const sprite_id_t id , const int z)
		: mId(id)
		, mZ(z) {
}

bool SelectPicking::Hit::operator>(const Hit& o) const {
	return mZ > o.mZ;
}

bool SelectPicking::Hit::operator<(const Hit& o) const {
	return mZ < o.mZ;
}

} // namespace ds
