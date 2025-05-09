#ifndef NAPL_PAN_IMPL_H
#define NAPL_PAN_IMPL_H

#include "samplebl.h"
#include "smputils.h"
#include "samplety.h"

//////////////////////////////////////////////
// stereo panning
// A stereo pan object receives stereo samples and performs panning
// on them by suppressing either the left signal (pan right) or the
// right signal (pan left).
//
template< typename mono_type>
class stereo_pan_mutator
{
public:
	typedef StereoSample< mono_type> sample_type;
	typedef int result_information_type;

	stereo_pan_mutator( )
		: m_pan_value( 0)
	{

	}

	void set_pan( short pan_value)
	{
		m_pan_value = pan_value;
	}

	inline void MutateHeader( stream_header &h)
	{
		// nop
	}

	sample_type operator()( const sample_type &input) const
	{
		if (m_pan_value< 0)
		{
            return { input.left(), fixed_damp( 32768 + m_pan_value, input.right())};
		}
		else
		{
            return { fixed_damp( 32767 - m_pan_value, input.left()), input.right()};
		}
	}

private:
	short m_pan_value;
};

template< typename mono_type>
class stereo_pan : public uniform_block_mutator< stereo_pan_mutator< mono_type> >
{
public:
	stereo_pan( short pan_value)
	{
		uniform_block_mutator< stereo_pan_mutator< mono_type> >::
			m_sample_mutator.set_pan( pan_value);
	};
};

#endif
