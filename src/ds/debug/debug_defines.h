#ifndef DS_DEBUG_DEBUGDEFINES_H_
#define DS_DEBUG_DEBUGDEFINES_H_

#ifdef _DEBUG
#include <Poco/Debugger.h>
#endif

/* Macros for various DEBUG-only conditions.
 ******************************************************************/
#ifdef _DEBUG
	#define DS_ASSERT(cnd)						if (!(cnd)) { Poco::Debugger::enter(); }
	#define DS_ASSERT_MSG(cnd, msg)				if (!(cnd)) { Poco::Debugger::enter(msg); }
	#define DS_VALIDATE(cnd, expr)				if (!(cnd)) { Poco::Debugger::enter(); expr; }
	// Expr1 is meant to be some sort of error string. Only triggered in dbg.
	#define DS_VALIDATE_2(cnd, expr1, expr2)	if (!(cnd)) { expr1; expr2; }
	// Useful when throwing in some testing code that you want to make sure never gets into release
	#define DS_DBG_CODE(code)					code
	// For when you want debug code in a header file
	#define DS_DBG_HDR(code)					code
#else
	#define DS_ASSERT(cnd)						(void*)0
	#define DS_ASSERT_MSG(cnd, msg)				(void*)0
	#define DS_VALIDATE(cnd, expr)				if (!(cnd)) { expr; }
	#define DS_VALIDATE_2(cnd, expr1, expr2)	if (!(cnd)) { expr2; }
	#define DS_DBG_CODE(op)						(void*)0
	#define DS_DBG_HDR(code)					;
#endif

/* OpenGL debugging
 ******************************************************************/
#ifdef _DEBUG
#define TURN_ON_REPORT_GL_ERRORS
#endif

/* OpenGL debugging -- impl
 ******************************************************************/
#ifdef TURN_ON_REPORT_GL_ERRORS
namespace ds {
  void			report_gl_errors();
}
#define DS_REPORT_GL_ERRORS()		ds::report_gl_errors();
#else
#define DS_REPORT_GL_ERRORS()		(void*)0
#endif

#endif // DS_DEBUG_DEBUGDEFINES_H_