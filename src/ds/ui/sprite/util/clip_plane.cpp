#include "clip_plane.h"
#include <cinder/gl/gl.h>
#include <cinder/Vector.h>
#include "ds/debug/debug_defines.h"

namespace {

//void planeEquation( double *eq, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3)
//{
//  eq[0] = (y1*(z2 – z3)) + (y2*(z3 – z1)) + (y3*(z1 – z2));
//  eq[1] = (z1*(x2 – x3)) + (z2*(x3 – x1)) + (z3*(x1 – x2));
//  eq[2] = (x1*(y2 – y3)) + (x2*(y3 – y1)) + (x3*(y1 – y2));
//  eq[3] = -((x1*((y2*z3) – (y3*z2))) + (x2*((y3*z1) – (y1*z3))) + (x3*((y1*z2) – (y2*z1))));
//}

}

namespace ds {
namespace ui {

void enableClipping( float x0, float y0, float x1, float y1 )
{
  glPushAttrib( GL_TRANSFORM_BIT | GL_ENABLE_BIT );
  //glEnable(GL_SCISSOR_TEST);
  //glScissor(x0, y0, x1, y1);
  ci::Vec3f clippingPoints[4];
  clippingPoints[0].set( x0, y0, 0 );
  clippingPoints[1].set( x0, y1, 0 );
  clippingPoints[2].set( x1, y1, 0 );
  clippingPoints[3].set( x1, y0, 0 );

  for (int i = 0; i < 4; ++i) {
    int j = (i+1) % 4;
    int k = (i+2) % 4;

    // Going clockwise around clipping points...
    ci::Vec3f edgeA = clippingPoints[i] - clippingPoints[j];
    ci::Vec3f edgeB = clippingPoints[j] - clippingPoints[k];

    // The edge-normal is found by first finding a vector perpendicular
    // to two consecutive edges.  Next, we cross that with the forward-
    // facing (clockwise) edge vector to get an inward-facing edge-
    // normal vector for that edge
    
    ci::Vec3f norm = -(edgeA.cross( edgeB )).cross(edgeA).normalized();

    // the four points we pass to glClipPlane are the solutions of the
    // equation Ax + By + Cz + D = 0.  A, B, and C are the normal, and
    // we solve for D. C is always zero for the 2D case however, in the
    // 3D case, we must use a three-component normal vector.
    
    float d = -norm.dot(clippingPoints[i]);

    DS_REPORT_GL_ERRORS();
    glEnable( GL_CLIP_PLANE0 + i );
    DS_REPORT_GL_ERRORS();
    GLdouble equation[4] = { norm.x, norm.y, norm.z, d };
    glClipPlane( GL_CLIP_PLANE0 + i, equation );
    DS_REPORT_GL_ERRORS();
  }
}

void disableClipping()
{
  glPopAttrib();
  //glDisable(GL_SCISSOR_TEST);
}

}
}
