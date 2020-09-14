#pragma once

#include <ds/app/event.h>

namespace ds {

class SomethingHappenedEvent : public ds::RegisteredEvent<SomethingHappenedEvent>{};

} // !namespace ds

