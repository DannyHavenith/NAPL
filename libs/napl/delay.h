#ifndef DELAY
#define DELAY
#include "samplebl.h"
#include <algorithm>
struct delay_base : public block_mutator, protected block_owner
{
    // maximum blocksize we're willing to spend on a block filled with zeros.
    enum { delay_block_size = 65536};

    delay_base( double delay_time_seconds, size_t samplesize, double post_seconds = 0)
        :m_delay_seconds( delay_time_seconds),
        m_samplesize( samplesize),
        m_post_seconds( post_seconds),
        m_producer_samples( 0),
        block_owner( delay_block_size)
    {
        // nop
    }

    // when asked for a header give that of our producer, but alter the
    // samplecount first
    //
    void GetStreamHeader( stream_header &h) override
    {
        m_pProducer->GetStreamHeader( h);


        m_delay_samples = static_cast<size_t>(h.samplerate * m_delay_seconds);
        m_post_samples = static_cast<size_t>(h.samplerate * m_post_seconds);

        h.numframes += (m_delay_samples + m_post_samples);
    }

    void LinkTo( block_producer_ptr p) override
    {
        if (p)
        {
            stream_header h;
            p->GetStreamHeader( h);
            m_producer_samples = h.numframes;
        }
        else
        {
            m_producer_samples = 0;
        }
        block_mutator::LinkTo( std::move( p));
    }


    sample_block RequestBlock( sampleno start, unsigned long num) override
    {

        if (start < m_delay_samples)
        {
            const size_t count = std::min( m_delay_samples - start, num);
            return get_delay_samples( count);
        }

        start -= m_delay_samples;

        if (start < m_producer_samples)
        {
            const size_t count = std::min( m_producer_samples - start, num);

            return m_pProducer->RequestBlock( start, count);
        }
        start -= m_producer_samples;

        return get_delay_samples( num);
    }

    void init_block( sample_block &block)
    {
        std::fill( block.buffer_begin(), block.buffer_begin() + block.buffer_size(), 0);
           //( block.buffer_begin(), 0, block.buffer_size());
    }

    sample_block get_delay_samples( size_t num_samples)
    {
        sample_block block( get_block());

        init_block( block);
        const auto block_samples = std::min( num_samples, block.size() / m_samplesize);

        block.m_end = block.m_start
                        + m_samplesize * block_samples;
        return block;
    }


private:
    double m_delay_seconds;
    double m_post_seconds;
    size_t m_delay_samples;
    size_t m_post_samples;
    size_t m_producer_samples;
    size_t m_samplesize;
};

template <typename sampletype>
struct delay: public delay_base
{
    delay( double delay_seconds, double post_seconds = 0)
        :delay_base( delay_seconds, sizeof( sampletype), post_seconds)
    {
    };

};


#endif /* DELAY */
