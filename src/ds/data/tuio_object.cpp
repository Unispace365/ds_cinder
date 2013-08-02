#include "ds/data/tuio_object.h"

namespace ds {

/**
 * \class ds::TuioObject
 */
TuioObject::TuioObject()
		: mObjectId(-1) {
}

TuioObject::TuioObject(const int objectId, const cinder::Vec2f& position)
		: mObjectId(objectId)
		, mPosition(position) {
}

} // namespace ds
