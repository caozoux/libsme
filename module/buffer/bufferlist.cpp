#if 0
#include "bufferlist.h"
#include <malloc.h>

template<class T>
T* CMObectList<T>::Create(void)
{
	T* a;
	a = (T*)malloc(sizeof(T));
	return a;
}

template<class T>
int  CMObectList<T>::Add(T *d)
{
	return 0;
}

template<class T>
int  CMObectList<T>::Del(T *d)
{
}

template<class T>
T * CMObectList<T>::Find(T *d)
{
	T *a;
	return a;
}

template<class T>
void  CMObectList<T>::Insert(T *d)
{
}

template<class T>
CMObectList<T>::CMObectList()
{
}

template<class T>
CMObectList<T>::~CMObectList()
{}
#endif
