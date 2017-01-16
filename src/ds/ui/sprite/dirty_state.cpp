#include "stdafx.h"

#include "ds/ui/sprite/dirty_state.h"

namespace ds {
namespace ui {

DirtyState newUniqueDirtyState()
{
	static int			NEXT_DS = 0;
	return DirtyState(NEXT_DS++);
}

namespace {
const DirtyState	GENERIC_DIRTY_  				= newUniqueDirtyState();
const DirtyState	INTERNAL_A_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_B_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_C_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_D_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_E_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_F_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_G_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_H_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_I_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_J_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_K_DIRTY_				= newUniqueDirtyState();
const DirtyState	INTERNAL_L_DIRTY_				= newUniqueDirtyState();
}

const DirtyState&	GENERIC_DIRTY           = GENERIC_DIRTY_;
const DirtyState&	INTERNAL_A_DIRTY        = INTERNAL_A_DIRTY_;
const DirtyState&	INTERNAL_B_DIRTY        = INTERNAL_B_DIRTY_;
const DirtyState&	INTERNAL_C_DIRTY        = INTERNAL_C_DIRTY_;
const DirtyState&	INTERNAL_D_DIRTY        = INTERNAL_D_DIRTY_;
const DirtyState&	INTERNAL_E_DIRTY        = INTERNAL_E_DIRTY_;
const DirtyState&	INTERNAL_F_DIRTY        = INTERNAL_F_DIRTY_;
const DirtyState&	INTERNAL_G_DIRTY        = INTERNAL_G_DIRTY_;
const DirtyState&	INTERNAL_H_DIRTY        = INTERNAL_H_DIRTY_;
const DirtyState&	INTERNAL_I_DIRTY        = INTERNAL_I_DIRTY_;
const DirtyState&	INTERNAL_J_DIRTY        = INTERNAL_J_DIRTY_;
const DirtyState&	INTERNAL_K_DIRTY        = INTERNAL_K_DIRTY_;
const DirtyState&	INTERNAL_L_DIRTY        = INTERNAL_L_DIRTY_;

} // namespace ui
} // namespace ds
