////////////////////////////////////////////////////////////////////////
//
// simpops.h - define simple operators on sampleblocks
//
////////////////////////////////////////////////////////////////////////
#ifndef _SIMPOPS_H_
#define _SIMPOPS_H_

#include "debugprint.h"
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



    // just relay the header request to our producer.
    void GetStreamHeader( stream_header &h) override
    {
        m_pProducer->GetStreamHeader( h);
        h.numframes = m_size;
    }

    sample_block RequestBlock( sampleno start, unsigned long num) override
    {

        // relay the block request
        return m_pProducer->RequestBlock( start + m_start, num);
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

    void LinkTo( block_producer_ptr p)
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
    void LinkTo( block_producer_ptr p, int channel)
    {
        assert( channel == 0 or channel == 1);

        if (channel == 0)
        {
            stream_header h;
            block_consumer::LinkTo( std::move( p));
            m_pProducer->GetStreamHeader( h);
            m_treshold = h.numframes;
        }
        else
        {
            m_pSecondProducer = CheckArchitecture(std::move( p));
        }
    }

    // just relay the header request to our producer.
    void GetStreamHeader( stream_header &h) override
    {
        stream_header h2;
        m_pProducer->GetStreamHeader( h);

        m_pSecondProducer->GetStreamHeader( h2);
        h.numframes += h2.numframes;

    }

    sample_block RequestBlock( sampleno start, unsigned long num) override
    {
        // relay the block request
        if (start < m_treshold)
        {
            return m_pProducer->RequestBlock( start, std::min( num, m_treshold - start));
        }
        else
        {
            return m_pSecondProducer->RequestBlock( start - m_treshold, num);
        }
    }

private:
    sampleno m_treshold;
    block_producer_ptr m_pSecondProducer;
};

#endif
