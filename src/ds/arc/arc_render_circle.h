#pragma once
#ifndef DS_ARC_ARCRENDERCIRCLE_H_
#define DS_ARC_ARCRENDERCIRCLE_H_

#include <string>
#include <unordered_map>
#include <cinder/Surface.h>
#include "ds/arc/arc.h"

namespace ds {
namespace arc {

class RenderCircleParams {
  public:
    RenderCircleParams();
};

/**
 * \class ds::arc::RenderCircle
 * \brief Given an arc, generate a circular image.
 */
class RenderCircle
{
  public:
    RenderCircle();

		bool						on(ci::Surface8u&, ds::arc::Arc&);
};

} // namespace arc
} // namespace ds

#endif // DS_ARC_ARCRENDER_H_