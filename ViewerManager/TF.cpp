#include "TF.h"

TF::TF(unsigned int size, unsigned int offset) 
	: m_size(size), m_offset(offset)
{
	m_tf = SAFE_ALLOC_1D(TF_BGRA, size);
}


TF::~TF(void) {
	SAFE_DELETE_ARRAY(m_tf);
}