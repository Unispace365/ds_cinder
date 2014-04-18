#include "select_picking.h"

namespace ds {

/**
 * \class ds::SelectPicking
 */
SelectPicking::SelectPicking() {
}

ds::ui::Sprite* SelectPicking::pickAt(const ci::Vec2f& pt, ds::ui::Sprite& root) {
std::cout << "SELECT!" << std::endl;
return nullptr;
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

//	root.drawServer();

//	glPopAttrib();

#if 0
	// Process Hits
	int numHits = glRenderMode( GL_RENDER );
	map< int, unsigned > depthIdMap;
	vector< int > hitIds;
	hitIds.reserve( numHits );


	GLuint *ptr = selectBuf;
	for ( int i=0; i<numHits; i++ ) {
		int numNamesInHit = *(ptr+0);
		unsigned z = *(ptr+1);
		int id = numNamesInHit > 0 ? *(ptr+3) : -1;

		depthIdMap[id] = z;
		hitIds.push_back(id);

		ptr += 3 + numNamesInHit;
	}

	std::map<int, BaseSprite*>			hitSprites;
	masterWindow->getAllSpritesForIds(hitIds, hitSprites);

	// Traverse backwards through the hit list
	BaseSprite *ret = 0;
	for (auto it=hitIds.rbegin(), end=hitIds.rend(); it != end; ++it) {
		BaseSprite *s = hitSprites[*it];

		// Determine if this hit has an anscestor that is pick-by-3d
		WindowSprite *curWindow = s->parent;
		while( curWindow ) {
			if ( curWindow->is3dPickingEnabled() ) {
				return findNearestPickHit( curWindow, hitIds, depthIdMap, hitSprites); 
			}
			curWindow = curWindow->parent;
		}

		// No 3dPickEnabled ancestor -- return this sprite
		ret = s;
		break;
	}

	return ret;
#endif
return nullptr;
}

} // namespace ds
