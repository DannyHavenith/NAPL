////////////////////////////////////////////////////////////////////////
//
// convendi.h - define a sample block processor that converts endiannes
// (i.e. it swaps byte order)
//
////////////////////////////////////////////////////////////////////////
#ifndef _CONVENDI_H_
#define _CONVENDI_H_

#include "samplebl.h"
#include "filetype.h"
#include "samplety.h"

template<typename sampletype>
struct mut_endian_converter
{
public:

	typedef sampletype sample_type;
	typedef int result_information_type;

	inline void MutateHeader( stream_header &h)
	{
		h.architecture ^= SH_ARCH_ENDIAN;
	}

	// swap all byte positions.
	sample_type operator()( const sample_type &input_sample)
	{
        sample_type sample = input_sample;
		char c;
		char *p = (char *) &sample;
		char *q = p + sizeof(sampletype) - 1;

		//
		// we expect this for loop to be completely eliminated
		//
		for (int n = sizeof(sampletype)/2; n; --n)
		{
			c = *p;
			*p++ = *q;
			*q-- = c;
		}
        return sample;
	}
};

// mono 8 is a special case, we convert from unsigned to signed
template <> struct mut_endian_converter<sampletype_m8>
{
public:

	typedef sampletype_m8 sample_type;
	typedef int result_information_type;

	inline void MutateHeader( stream_header &h)
	{
		h.architecture ^= SH_ARCH_ENDIAN;
	}

	// swap all byte positions.
	sampletype_m8 operator()( sampletype_m8 p)
	{
		return ((short)(char)p) + 128;
	}
};

template< typename sampletype>
class endian_converter: public uniform_block_mutator< mut_endian_converter< sampletype> >
{
	virtual block_producer *CheckArchitecture( block_producer *pP)
	{
		//
		// whatever the block_producer, it's fine by me...
		//
		return pP;
	}
};

#endif //_CONVENDI_H_
