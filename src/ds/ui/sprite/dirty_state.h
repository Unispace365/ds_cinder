#pragma once
#ifndef DS_UI_SPRITE_DIRTYSTATE_H_
#define DS_UI_SPRITE_DIRTYSTATE_H_

#include "ds/util/bit_mask.h"

namespace ds { namespace ui {

	typedef BitMask DirtyState;

	/**
	 * \brief Create a new, globally unique dirty state.  This is meant to be run statically
	 * or in the main thread.  Typical usage might look like:
	 * namespace {
	 * const ds::DirtyState	POSITION_DIRTY = ds::newUniqueDirtyState();
	 * }
	 */
	DirtyState newUniqueDirtyState();

	/**
	 * \brief Sprites can use the following for a non-specific
	 * dirty message that will just cause a packet send.
	 */
	extern const DirtyState& GENERIC_DIRTY;
	/**
	 * \brief Sprites can use the following for entirely internal dirty states.
	 * To work, the sprite should have no class above or below it in the sprite hierarchy
	 * that uses the same states.  So just use for leaf sprites.
	 * This is done because I'm hitting the limit for number of dirty states, and
	 * increasing the bitmask to handle more is a little annoying.
	 */
	extern const DirtyState& GENERIC_DIRTY;
	extern const DirtyState& INTERNAL_A_DIRTY;
	extern const DirtyState& INTERNAL_B_DIRTY;
	extern const DirtyState& INTERNAL_C_DIRTY;
	extern const DirtyState& INTERNAL_D_DIRTY;
	extern const DirtyState& INTERNAL_E_DIRTY;
	extern const DirtyState& INTERNAL_F_DIRTY;
	extern const DirtyState& INTERNAL_G_DIRTY;
	extern const DirtyState& INTERNAL_H_DIRTY;
	extern const DirtyState& INTERNAL_I_DIRTY;
	extern const DirtyState& INTERNAL_J_DIRTY;
	extern const DirtyState& INTERNAL_K_DIRTY;
	extern const DirtyState& INTERNAL_L_DIRTY;

}} // namespace ds::ui

#endif // DS_UI_SPRITE_DIRTYSTATE_H_
