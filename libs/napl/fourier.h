/***********************************************************************************
	Fourier.h - fourier transforms.


***********************************************************************************/

#ifndef FOURIER
#define FOURIER
#define nsp_UsesAll
#include "nsp.h"

namespace
{
	//
	// a crude but effective enough power of two...
	size_t power2( size_t order)
	{
		if (order == 0) return 1;
		size_t half = power2( order / 2);
		size_t result = half * half;
		if (order & 1)
		{
			result *= 2;
		}
		return result;
	}

	//
	// All of these overloaded functions translate a signal of 2 ^ order real values into
	// the FFT in an array of 2^order complex values.
	//
	inline void RealFFT( float *sample_ptr, size_t order, unsigned char *output_ptr)
	{
		nspsRealFftlNip( sample_ptr, (float *) output_ptr, order, NSP_Forw | NSP_OutRCPack); /* real values, single precision */
		nspcbConjExtend1( (SCplx *) output_ptr , power2( order - 1) + 1); /* complex values; single precision */
	}

	inline void RealFFT( double *sample_ptr, size_t order, unsigned char *output_ptr)
	{
		nspdRealFftlNip( sample_ptr, (double *) output_ptr, order, NSP_Forw | NSP_OutRCPack); /* real values, single precision */
		nspzbConjExtend1( (DCplx *) output_ptr , power2( order - 1) + 1); /* complex values; single precision */
	}

	inline void RealFFT( short *sample_ptr, size_t order, unsigned char *output_ptr)
	{
		int scale;
		nspwRealFftlNip( sample_ptr, (short *) output_ptr, order, NSP_Forw | NSP_OutRCPack, NSP_AUTO_SCALE, &scale); /* real values, single precision */
		nspvbConjExtend1( (WCplx *) output_ptr , power2( order - 1) + 1); /* complex values; single precision */
	}
}

template< class sample_t>
struct fft : public block_mutator, private block_owner
{
	typedef sample_t sample_type;
	typedef StereoSample< sample_t> output_sample_type;

	fft( size_t order)
		:block_owner( (sizeof output_sample_type * power2( order)) / 2 + 1),
		m_order( order)
	{
		// nop
	}

	virtual void Seek( sampleno start)
	{
		assert( false);
	}

	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num)
	{
		return m_pProducer->RequestBlock( *this, start, num);
	}

	// when asked for a header give that of our producer, but alter the
	// domain from time to frequency
	//
	virtual void GetStreamHeader( stream_header &h)
	{
		m_pProducer->GetStreamHeader( h);
		h.architecture |= ARCHITECTURE_FREQUENCY_DOMAIN;
	}

	virtual void ReceiveBlock( const sample_block &b)
	{
		sample_container< sample_type> container( b);
		sample_container< sample_type>::iterator first_sample_it = container.begin();
		int block_size = power2( m_order);

		while (container.end() - first_sample_it >= block_size)
		{
			RealFFT( first_sample_it, m_order, m_block.m_start);
			m_pConsumer->ReceiveBlock( m_block);
			first_sample_it += block_size;
		}
	}

protected:
	size_t m_order;
	virtual unsigned long GetArchitecture()
	{
		return LOCAL_ARCHITECTURE | ARCHITECTURE_FREQUENCY_DOMAIN;
	}
private:
};



#endif /* FOURIER */
