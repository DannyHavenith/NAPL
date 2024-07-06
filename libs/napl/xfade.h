////////////////////////////////////////////////////////////////////////
//
// xfade.h - definition of the cross-fader object
//
////////////////////////////////////////////////////////////////////////

#ifndef _XFADE_H_
#define _XFADE_H_

#include "samplebl.h"

template< typename sampletype>
struct mut_xfade
{
	typedef sampletype sample_type;

	inline void MutateHeader( stream_header &result, const stream_header &left, const stream_header &right)
	{
		m_currentLevel = 0;
		m_step = (0x7fffffff) / (result.numframes - 1);
	}

	inline const sampletype Mutate(
		const sampletype &left, const sampletype &right)
	{
		long bal= (long)(m_currentLevel >> 16);

		m_currentLevel += m_step;

		return fixed_balance( static_cast<unsigned short>(bal), right, left);

	}

protected:
	unsigned long m_currentLevel;
	unsigned long m_step;
};

template <typename sampletype>
struct xfader : public binary_block_mutator< mut_xfade< sampletype> >
{
};

#endif
