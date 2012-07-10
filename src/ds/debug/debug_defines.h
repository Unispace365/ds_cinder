#pragma once
#ifndef DS_DEBUG_DEBUGDEFINES_H_
#define DS_DEBUG_DEBUGDEFINES_H_

/* Macros for various DEBUG-only conditions.
 ******************************************************************/
#ifdef _DEBUG
	#define DS_VALIDATE(cnd, expr)			if (!(cnd)) { assert(false); expr; }
	// Useful when throwing in some testing code that you want to make sure never gets into release
	#define DS_DBG_CODE(code)				code
	// For when you want debug code in a header file
	#define DS_DBG_HDR(code)				code
#else
	#define DS_VALIDATE(cnd, expr)			if (!(cnd)) { expr; }
	#define DS_DBG_CODE(op)					(void*)0
	#define DS_DBG_HDR(code)				;
#endif

#endif // DS_DEBUG_DEBUGDEFINES_H_