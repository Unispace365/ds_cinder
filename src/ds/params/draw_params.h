#pragma once
#ifndef DS_DRAW_PARAMS_H
#define DS_DRAW_PARAMS_H

namespace ds {

/**
 * \class DrawParams
 * \brief Provided to sprites for draw()ing functions.
 */
class DrawParams {
public:
	DrawParams();
	float mParentOpacity;
};

} // namespace ds

#endif
