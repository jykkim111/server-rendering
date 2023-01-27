#pragma once
#include "global.h"
class TF
{
public:
	explicit TF(unsigned int size, unsigned int offset);
	~TF(void);

public: 
	inline unsigned int size()		{ return m_size; };
	inline unsigned int offset()	{ return m_offset; };
	inline TF_BGRA* getTF()			{ return m_tf; };


private:
	unsigned int m_size;
	unsigned int m_offset;
	TF_BGRA* m_tf;



};

