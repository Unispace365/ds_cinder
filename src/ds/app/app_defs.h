#pragma once
#ifndef DS_APP_APPDEFS_H_
#define DS_APP_APPDEFS_H_

namespace ds {

typedef int                 sprite_id_t;
// System-defined illegal sprite
static const sprite_id_t    EMPTY_SPRITE_ID = 0;
// System-defined root sprite, there is always one, and apps can't manage it.
// All other sprites descend from this sprite.
//static const sprite_id_t    ROOT_SPRITE_ID = -1;
//static const sprite_id_t    ROOT_PERSPECTIVE_SPRITE_ID = -2;

// Blob termination for network traffic
static const char           TERMINATOR_CHAR = 0;

} // namespace ds

#endif // DS_APP_APPDEFS_H_