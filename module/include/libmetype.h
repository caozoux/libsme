#ifndef __LIBME_TYPE_H__
#define __LIBME_TYPE_H__

#ifndef __PLATFORM_BASE__

#define __PLATFORM_BASE__
#ifndef LINUX_HOST
#include<windows.h>

#else

//typedef unsigned long DWORD
#define HINSTANCE unsigned int
#define DWORD unsigned long
#define BOOL bool
#define NULL ((void *)0)
#define VOID void

#endif

#endif

