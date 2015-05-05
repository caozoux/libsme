#ifndef __LIBME_TYPE_H__
#define __LIBME_TYPE_H__


#define __PLATFORM_BASE__
#ifdef WIN_HOST
#include<windows.h>

#else

//typedef unsigned long DWORD
#define HINSTANCE unsigned int
#define DWORD unsigned long
#define BOOL bool
#define NULL ((void *)0)
#define VOID void

typedef unsigned int u32;
typedef unsigned short int u16;

#endif

#endif

