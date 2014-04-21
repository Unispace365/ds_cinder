#include "picking.h"

namespace ds {

/**
 * \class ds::Picking
 */
Picking::Picking() {
}

Picking::~Picking() {
}

void Picking::setWorldSize(const ci::Vec2f& ws) {
	mWorldSize = ws;
}

} // namespace ds
