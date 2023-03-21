#include "stdafx.h"

#include "picking.h"

namespace ds {

/**
 * \class Picking
 */
Picking::Picking() {}

Picking::~Picking() {}

void Picking::setWorldSize(const ci::vec2& ws) {
	mWorldSize = ws;
}

} // namespace ds
