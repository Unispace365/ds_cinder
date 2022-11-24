#pragma once

#include <ds/app/event.h>

namespace fullstarter {

class SomethingHappenedEvent : public ds::RegisteredEvent<SomethingHappenedEvent> {};

} // namespace fullstarter
