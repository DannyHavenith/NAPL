////////////////////////////////////////////////////////////////////////////////
//
// Resample.h - definition of the resampler object
//
#ifndef _RESAMPLE_H_
#define _RESAMPLE_H_

#include "samplebl.h" // include sample block definitions
#include "smputils.h"
#include "general_resampler.h"
////////////////////////////////////////////////////////////////////////////////
//
// The resample object resamples a sample stream. It converts from one sampling
// frequency to another e.g. from 44100Hz to 56000Hz.
//
// The resampler uses a heavily mutilated Bressenham's algorithm to get a close
// aproximation of the original sound data.
//

template <typename sampletype>
class resampler: public general_resampler
{
public:

	resampler( unsigned long outfreq, unsigned long infreq, bool lie)
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

	sample_block RequestBlock( sampleno start, unsigned long num) override
	{
        return get_block();

        // TODO: implement resampler again

        /*
		sample_state = request_related_state();

		sample_block destination_block( get_block());

		InitSendBlock( destination_block);

		sample_state.m_inputAdvance = 1;
		sample_state.m_accumulator = (((unsigned long) start) * m_infreq) % m_outfreq;

		// calculate the first and last sample that we need
		sampleno input_start = static_cast<sampleno>((int64bit( start) * m_infreq) / int64bit( m_outfreq));
		sampleno input_last = static_cast<sampleno>((int64bit( start + num -1) * m_infreq +  m_outfreq - 1)/ m_outfreq);

		sample_state.m_requestedSamples = input_last - input_start + 1;
		sample_state.m_outputSamples = num;

		// relay the block request
		const auto source_block =  m_pProducer->RequestBlock( *this, input_start, sample_state.m_requestedSamples);

		// receiving a new block. copy relevant information into local variables.

		// pCurrentSample always points to the rightmost sample (= the newest sample)
		// that is of influence on the output-sample.
		sampletype *pCurrentSample =
			reinterpret_cast< sampletype *>( source_block.m_start);
		sampletype *pEnd=
			reinterpret_cast< sampletype *>( source_block.m_end);

		sample_state.m_requestedSamples -= (pEnd - pCurrentSample);

		// return to the state we had at the end of the last block
		unsigned long accumulator = sample_state.m_accumulator;
		unsigned long inputAdvance = sample_state.m_inputAdvance;
		sampletype previousSample = sample_state.m_previousSample;

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
						sample_state.m_previousSample = *(pEnd - 1);

					if (sample_state.m_requestedSamples)
					{
						// there's, still samples to go. now store our state.
						sample_state.m_inputAdvance = pCurrentSample - pEnd;
						sample_state.m_accumulator = accumulator;
					}
					else
					{
						// send the last block
						//
						while (sample_state.m_outputSamples)
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
        */
	}


private:
/*
	inline void EmitSample( const sampletype &s)
	{
		if (sample_state.m_outputSamples)
		{
		    *sample_state.m_pOutputSample++ = s;
			if (sample_state.m_pOutputSample >= sample_state.m_pEndOutput)
			{
				// our block is full, send it
				SendBlock();
				sample_state.m_pOutputSample = reinterpret_cast<sampletype *>(sample_state.m_block_ptr->m_start);

			}
			--sample_state.m_outputSamples;
		}
	}

	inline void SendBlock()
	{
		sample_state.m_block_ptr->m_end = (unsigned char *)sample_state.m_pOutputSample;
		sample_state.m_pOutputSample = (sampletype *)sample_state.m_block_ptr->m_start;
		if (sample_state.m_block_ptr->m_end - sample_state.m_block_ptr->m_start)
		{
			m_pConsumer->ReceiveBlock( *m_state.m_block_ptr);
		}

	}

	inline void InitSendBlock( sample_block &block)
	{
		block.m_buffer_ptr->m_size = (block.buffer_size()/sizeof( sampletype)) * sizeof( sampletype);
		sample_state.m_pEndOutput = (sampletype *)(block.m_end = block.buffer_begin() + block.buffer_size());
		sample_state.m_pOutputSample = (sampletype *)block.m_start;
	}
*/
	unsigned long m_infreq;
	unsigned long m_outfreq;
	bool m_lieAboutSampleRate;

	struct request_related_state
	{
		request_related_state()
			:m_accumulator( 0),
			m_inputAdvance( 0),
			m_requestedSamples( 0),
			m_block_ptr(0),
			m_pEndOutput( 0),
			m_pOutputSample(0),
			m_outputSamples(0)
		{
		}

		unsigned long m_accumulator;
		unsigned long m_inputAdvance;
		unsigned long m_requestedSamples;
		unsigned long m_outputSamples;
		sampletype *m_pOutputSample;
		sampletype *m_pEndOutput;
		sampletype m_previousSample;
		sample_block *m_block_ptr;
	};

	request_related_state sample_state;
};

#endif
