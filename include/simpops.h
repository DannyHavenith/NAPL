////////////////////////////////////////////////////////////////////////
//
// simpops.h - define simple operators on sampleblocks
//
////////////////////////////////////////////////////////////////////////
#ifndef _SIMPOPS_H_
#define _SIMPOPS_H_

#include "samplebl.h"

///////////////////////////////////////////////////////////////////////////////////
//
// a cut_mutator takes a specific part of a sample and leaves out the rest
//
class basic_cut_mutator: public block_mutator
{

public:
	basic_cut_mutator()
		:m_source_numframes(0)
	{
		// nop
	}

	void SetCut( sampleno start, long num)
	{
		if (start < 0) start = 0;
		if (num < 0) num = 0;

		long maxnum = get_source_numframes() - start;

		if (maxnum < 0)
		{
			start = num = 0;
		}
		else
		{
			if (num > maxnum || num == 0) num = maxnum;
		}

		m_start = start;
		m_size = num;
	}

	virtual void Seek( sampleno start)
	{
		m_pProducer->Seek( m_start + start);
	}

	// just relay the header request to our producer.
	virtual void GetStreamHeader( stream_header &h)
	{
		m_pProducer->GetStreamHeader( h);
		h.numframes = m_size;
	}

	virtual inline block_result RequestBlock( block_consumer &c, sampleno start, unsigned long num)
	{
		state_saver<block_consumer *> save( m_pConsumer);
		m_pConsumer = &c;

		// relay the block request
		return m_pProducer->RequestBlock( c, start + m_start, num);
	}

	virtual void ReceiveBlock( const sample_block &b)
	{
		// pass the block.
		m_pConsumer->ReceiveBlock( b);
	}

protected:
	sampleno get_source_numframes()
	{
		sampleno result = 0;

		if (!m_source_numframes)
		{
			if (m_pProducer)
			{
				stream_header h;
				m_pProducer->GetStreamHeader( h);
				m_source_numframes = h.numframes;
			}
		}

		return m_source_numframes;
	}

private:
	sampleno m_source_numframes;
	sampleno m_start;
	long m_size;
};

//
// a cut mutator is a static cut that does not change
class cut_mutator : public basic_cut_mutator
{
};

// a stream iterator is a cut_mutator that can be shifted from one position to the next
class stream_cut_iterator : public basic_cut_mutator
{
public:
	stream_cut_iterator( sampleno window_width, sampleno jump_size = 0)
		: m_window_width( window_width),
		m_jump_size( jump_size),
		m_current_pos( 0)
	{
		if (jump_size == 0)
		{
			m_jump_size = window_width;
		}
		//set_window();
	}

	bool at_end()
	{
		return m_current_pos * m_jump_size >= get_source_numframes();
	}

	bool next()
	{
		++m_current_pos;
		set_window();
		return !at_end();
	}

	void reset()
	{
		m_current_pos = 0;
		set_window();
	}

	void LinkTo( block_producer *p)
	{
		block_consumer::LinkTo( p);
		set_window();
	}

private:
	void set_window()
	{
		SetCut( m_current_pos * m_jump_size, m_window_width);
	}
	unsigned long m_current_pos;
	sampleno m_window_width;
	sampleno m_jump_size;
};

///////////////////////////////////////////////////////////////////////////////////
//
// a paste_mutator concatenates two samples
//
///////////////////////////////////////////////////////////////////////////////////
class paste_mutator: public block_mutator
{
public:
	void LinkTo( block_producer *p, int channel)
	{
        assert( channel >= 0 and channel <= 1);
		if (channel == 0)
		{
			stream_header h;
			block_consumer::LinkTo( p);
			m_pProducer->GetStreamHeader( h);
			m_treshold = h.numframes;
		}

		if (channel == 1)
		{
			m_pSecondProducer = p;
			m_pSecondProducer->LinkToConsumer( this);
		}
	}

	virtual void Seek( sampleno start)
	{
		if (start < m_treshold)
			m_pProducer->Seek( start);
		else
			m_pSecondProducer->Seek( start - m_treshold);
	}

	// just relay the header request to our producer.
	virtual void GetStreamHeader( stream_header &h)
	{
		stream_header h2;
		m_pProducer->GetStreamHeader( h);

		m_pSecondProducer->GetStreamHeader( h2);
		h.numframes += h2.numframes;

	}

	virtual inline block_result RequestBlock( block_consumer &c, sampleno start, unsigned long num)
	{
		// this is not quite thread-safe. This object keeps the requesting consumer in its
		// state until the next call to RequestBlock
		//
		// each thread should have it's own mutator...
		state_saver< block_consumer *> save( m_pConsumer);
		m_pConsumer = &c;

		// relay the block request
		if (start < m_treshold)
		{
			unsigned long num1 = m_treshold - start;
			if (num1 > num) num1 = num;
			m_pProducer->RequestBlock( c, start, num1);
			num -= num1;
			start = 0;
		}
		else
		{
			start -= m_treshold;
		}

		if (num)
		{
			m_pSecondProducer->RequestBlock( c, start, num);
		}
		return true;
	}

	virtual void ReceiveBlock( const sample_block &b)
	{
		// pass the block.
		m_pConsumer->ReceiveBlock( b);
	}

private:
	sampleno m_treshold;
	block_producer *m_pSecondProducer;
};

/**
 * This class is wrapper around the paste mutator that keeps
 * shared pointers to the producers it is linked to.
 */
class owning_paste_mutator: public block_mutator
{
    public:
        using block_producer_ptr = std::shared_ptr< block_producer>;
        owning_paste_mutator( std::unique_ptr<paste_mutator> paster)
            :paster( std::move( paster))
        {
            // nop
        }

        void LinkTo( block_producer_ptr p, int channel)
        {
            assert( channel >= 0 and channel <= 1);
            paster->LinkTo( p.get(), channel);
            producers[channel] = std::move( p);
        }

        void Seek( sampleno start) override
        {
            paster->Seek( start);
        }

        void GetStreamHeader( stream_header &h) override
        {
            paster->GetStreamHeader( h);
        }

        block_result RequestBlock( block_consumer &c, sampleno start, unsigned long num) override
        {
            return paster->RequestBlock( c, start, num);
        }

        void ReceiveBlock( const sample_block &b) override
        {
            paster->ReceiveBlock( b);
        }

    private:
        std::unique_ptr<paste_mutator> paster;
        std::vector<std::shared_ptr<block_producer>> producers{2};
};

#endif
