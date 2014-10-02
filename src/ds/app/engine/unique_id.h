#pragma once
#ifndef DS_APP_ENGINE_UNIQUEID_H_
#define DS_APP_ENGINE_UNIQUEID_H_

#include <string>

namespace ds {

// Answer a unique ID that includes the machine name.
// This is in its own file because boost and cinder were
// colliding.
std::string			get_unique_id();

} // namespace ds

#endif