#ifndef __MELIB_BUFFER_LIST__
#define __MELIB_BUFFER_LIST__
#include <list>
#include <malloc.h>
using namespace std;

class CMBBufferList{

};

template<class T> class CMObectList {
	public:
		list<T*> mData;
	public:
		T *Create(void);
		int Add(T *d);
		int Del(T *d);
		T* Find(T *d);
		void Insert(T *d);
		void Dump(void);
		CMObectList();
		~CMObectList();
};

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
	mData.push_back(d);
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
void CMObectList<T>::Dump(void)
{
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
