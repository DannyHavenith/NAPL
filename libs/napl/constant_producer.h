#ifndef CONSTANT_PRODUCER_H
#define CONSTANT_PRODUCER_H
#include "samplebl.h"

#include <algorithm>

template<typename sample_type>
class constant_producer : public block_producer, private block_owner
{
private:
    stream_header m_header;

    void init_block( sample_block &block)
    {
        sample_container< sample_type> cont( block);
        std::fill( cont.begin(), cont.end(), sampletraits< sample_type>::get_middle());
    }
public:
    constant_producer( stream_header h)
        :m_header( h)
    {
        m_header.architecture = LOCAL_ARCHITECTURE;
        m_header.numchannels = sampletraits<sample_type>::get_num_channels();
        m_header.samplesize = h.samplesize;
    }

    sample_block RequestBlock( sampleno /*ignored*/, unsigned long num) override
    {
        sample_block block( get_block());
        block.Truncate( num * sizeof( sample_type));

        init_block( block);

        return block;
    }

    void GetStreamHeader( stream_header &h) override
    {
        h = m_header;
    }

};
#endif // CONSTANT_PRODUCER_H
