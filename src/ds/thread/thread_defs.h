#pragma once
#ifndef DS_THREAD_THREADDEFS_H_
#define DS_THREAD_THREADDEFS_H_

#ifdef _DEBUG
// #define		DS_THREAD_DEBUG_IS_ON		(1)
#endif

#ifdef DS_THREAD_DEBUG_IS_ON
#define DS_DBG_THREAD_CODE(x) (x)
#else
#define DS_DBG_THREAD_CODE(x) (void*)0
#endif

#endif // DS_THREAD_THREADDEFS_H_
