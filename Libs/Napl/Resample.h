////////////////////////////////////////////////////////////////////////////////
//
// Resample.h - definition of the resampler object
//
#ifndef _RESAMPLE_H_
#define _RESAMPLE_H_

#include "samplebl.h" // include sample block definitions
#include "smputils.h"

////////////////////////////////////////////////////////////////////////////////
//
// The resample object resamples a sample stream. It converts from one sampling
// frequency to another e.g. from 44100Hz to 56000Hz.
//
// The resampler uses a heavily mutilated Bressenham's algorithm to get a close
// aproximation of the original sound data.
//
class general_resampler : public block_mutator, public block_owner
{
public:
	virtual void Init(unsigned long outfreq, unsigned long infreq, bool lie) = 0;
};


template <typename sampletype>
class resampler: public general_resampler
{
public:

	resampler( unsigned long outfreq, unsigned long infreq, bool lie)
		:m_outputSamples(0)
	{
		Init( outfreq, infreq, lie);
	}

	virtual void Init(unsigned long outfreq, unsigned long infreq, bool lie)
	{

		// if lie is true (sounds weird), we will not tell our consumer that we 
		// have altered the samplerate.
		// this results in a faster or slower playing sample.
		m_outfreq = outfreq;
		m_infreq = infreq;
		m_lieAboutSampleRate = lie;

		InitSendBlock();
	}

	virtual void Seek( sampleno start)
	{
		m_pProducer->Seek( start);
	}

	// when asked for a header give that of our producer, but alter the
	// samplerate first
	//
	virtual void GetStreamHeader( stream_header &h)
	{
		m_pProducer->GetStreamHeader( h);

		// if infreq has not been specified, we take the real input frequency
		if (!m_infreq) m_infreq = (unsigned short)h.samplerate;

		if (!m_lieAboutSampleRate) h.samplerate = m_outfreq;

		// calculate the new number of samples
		h.numframes = (unsigned long)(((int64bit)h.numframes * m_outfreq) / (int64bit)m_infreq);
	}

	virtual inline block_result RequestBlock( block_consumer &c, sampleno start, unsigned long num)
	{ 
		typedef state_saver<unsigned long> long_saver;

		state_saver<block_consumer *> save( m_pConsumer);
		m_pConsumer = &c;

		sampleno input_start;
		sampleno input_last;

		InitSendBlock();
		long_saver save2( m_inputAdvance);
		m_inputAdvance = 1;

		long_saver save3( m_accumulator);
		m_accumulator = (unsigned long( start) * m_infreq) % m_outfreq;

		// the first sample that we need
		input_start = static_cast<sampleno>((int64bit( start) * m_infreq) / int64bit( m_outfreq));

		// the last sample that we need
		input_last = static_cast<sampleno>((int64bit( start + num -1) * m_infreq +  m_outfreq - 1)/ m_outfreq);


		long_saver save4( m_requestedSamples);
		long_saver save5( m_outputSamples);
		m_requestedSamples = input_last - input_start + 1;
		m_outputSamples = num;

		// relay the block request
		return m_pProducer->RequestBlock( *this, input_start, m_requestedSamples);
	}

	virtual void ReceiveBlock( const sample_block &b)
	{
		// receiving a new block. copy relevant information into local variables.

		// pCurrentSample always points to the rightmost sample (= the newest sample)
		// that is of influence on the output-sample.
		sampletype *pCurrentSample = 
			reinterpret_cast< sampletype *>( b.m_start);
		sampletype *pEnd= 
			reinterpret_cast< sampletype *>( b.m_end);

		m_requestedSamples -= (pEnd - pCurrentSample);

		// return to the state we had at the end of the last block
		unsigned long accumulator = m_accumulator;
		unsigned long inputAdvance = m_inputAdvance;
		sampletype previousSample = m_previousSample;
		
		while (true)
		{
			if (inputAdvance)
			{
				// inputAdvance is the number of samples we advance in the input
				pCurrentSample += inputAdvance;
				if (pCurrentSample >= pEnd)
				{
					// we're past the end of the block now. See if we're exactly at the end
					// if so, we need to store the last sample for calculations in the next 
					// block.
					if (pCurrentSample == pEnd)
						m_previousSample = *(pEnd - 1);

					if (m_requestedSamples)
					{
						// there's, still samples to go. now store our state.
						m_inputAdvance = pCurrentSample - pEnd;
						m_accumulator = accumulator;
					}
					else 
					{
						// send the last block
						// 
						while (m_outputSamples)
						{
							EmitSample( *(pEnd - 1));
						}
						SendBlock();
					}

					return;
				}
				previousSample = *(pCurrentSample - 1);
			}
			sampletype s = balance( (unsigned short)(m_outfreq - accumulator), (unsigned short)accumulator, previousSample, *pCurrentSample);
			EmitSample( s);
			accumulator += m_infreq;
			inputAdvance = accumulator / m_outfreq;
			accumulator %= m_outfreq;
		}
	}


private:
	inline void EmitSample( const sampletype &s)
	{
		if (m_outputSamples)
		{
		    *m_pOutputSample++ = s;
			if (m_pOutputSample >= m_pEndOutput)
			{
				// our block is full, send it
				SendBlock();
				m_pOutputSample = reinterpret_cast<sampletype *>(m_block.m_start);

			}
			--m_outputSamples;
		}
	}

	inline void SendBlock()
	{
		m_block.m_end = (unsigned char *)m_pOutputSample;
		m_pOutputSample = (sampletype *)m_block.m_start;
		if (m_block.m_end - m_block.m_start)
		{
			m_pConsumer->ReceiveBlock( m_block);
		}

	}

	inline void InitSendBlock()
	{

		m_block.m_buffer_ptr->m_size = (m_block.buffer_size()/sizeof sampletype) * sizeof sampletype;
		m_pEndOutput = (sampletype *)(m_block.m_end = m_block.buffer_begin() + m_block.buffer_size());
		m_pOutputSample = (sampletype *)m_block.m_start;
	}

	unsigned long m_infreq;
	unsigned long m_outfreq;

	unsigned long m_accumulator;
	unsigned long m_inputAdvance;
	unsigned long m_requestedSamples;
	unsigned long m_outputSamples;
	bool m_lieAboutSampleRate;

	sampletype *m_pOutputSample;
	sampletype *m_pEndOutput;

	sampletype m_previousSample;
};

#endif