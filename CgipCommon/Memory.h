#pragma once
#include <iostream>
#include "../CgipException/CgipException.h"

extern "C++"
template <typename TYPE>
__forceinline TYPE * allocate_1D(size_t size) {
	TYPE* p;
	p = new (std::nothrow) TYPE[size];
	if (p == NULL) {
		throw cgip::CgipSlotNullException();
	}
	return p;
}

extern "C++"
template <typename TYPE>
__forceinline TYPE * allocate_2D(size_t row, size_t col) {
	TYPE* p;
	p = new (std::nothrow) TYPE[row * col];
	if (p == NULL) {
		throw cgip::CgipSlotNullException();
	}
	return p;
}

extern "C++"
template <typename TYPE>
__forceinline TYPE * *allocate_3D(size_t width, size_t height, size_t depth) {
	TYPE** p;
	p = new (std::nothrow) TYPE * [depth];
	if (p == NULL) {
		throw cgip::CgipSlotNullException();
	}
	for (int i = 0; i < depth; i++) {
		p[i] = new (std::nothrow) TYPE[width * height];
		if (p[i] == NULL) {
			throw cgip::CgipSlotNullException();
		}
	}
	return p;
}

/*
	MACRO : safe allocation of memory.
	1D & 2D & 3D
*/
#define SAFE_ALLOC_1D(TYPE, size) allocate_1D<TYPE>((size))
#define SAFE_ALLOC_2D( TYPE, row, col)	allocate_2D<TYPE>((row), (col))
#define SAFE_ALLOC_3D( TYPE, width, height, depth)	allocate_3D<TYPE>((width), (height), (depth))
/*
	MACRO : safe deallocation of memory.
*/
#define SAFE_DELETE_OBJECT(var) { if(var) {delete var; var = nullptr;} }
#define SAFE_DELETE_ARRAY(var) { if(var) {delete [] var; var = nullptr;} }
#define SAFE_DELETE_VOLUME(p, depth)	{ if(p) {for (int i=0;i<depth;i++)	SAFE_DELETE_ARRAY(p[i]);} SAFE_DELETE_ARRAY(p);}

#define MEMSET_ZERO_VOLUME(TYPE, p, depth, szSlice)	{ if(p) {for (int i=0;i<depth;i++)			memset(p[i],0,sizeof(TYPE)*szSlice);} }
#define MEMCPY_VOLUME(TYPE, dst, src, depth, szSlice)	{ if(dst) {for (int i=0;i<depth;i++)	memcpy(dst[i],src[i],sizeof(TYPE)*szSlice);} }