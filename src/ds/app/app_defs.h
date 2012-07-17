#pragma once
#ifndef DS_APP_APPDEFS_H_
#define DS_APP_APPDEFS_H_

namespace ds {

typedef int                 sprite_id_t;
static const sprite_id_t    EMPTY_SPRITE_ID = 0;

} // namespace ds

#if 0
// This is currently and hopefully permanently obsolete

// Three types of compile modes:
// Server contains the logic for managing all the views, and is what would generally be thought of
// as the application framework.
// Client is responsible for drawing the model.
// ServerClient puts both of them together as a single app.
//#define			DS_PLATFORM_SERVER			(1)
//#define			DS_PLATFORM_CLIENT			(1)
#define			DS_PLATFORM_SERVERCLIENT	(1)

#endif

#endif // DS_APP_APPDEFS_H_