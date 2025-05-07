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

    virtual block_result RequestBlock( block_consumer &consumer, sampleno /*ignored*/, unsigned long num)
    {
        block_handle h( this); // releases the block on exit
        sample_block &block( h.get_block());

        init_block( block);

        while (num* sizeof( sample_type) > (block.buffer_size()))
        {
            block.m_end = block.m_start + sizeof( sample_type) * (block.buffer_size() / sizeof(sample_type));
            consumer.ReceiveBlock( block);
            num -= (block.buffer_size() / sizeof( sample_type));
        }

        block.m_end = block.m_start + sizeof( sample_type) * num;
        const sample_container<sample_type> cont( block);
        assert(std::all_of( cont.begin(), cont.end(), [](sample_type sample) {
            return sample == sampletraits<sample_type>::get_middle();
        }));
        consumer.ReceiveBlock( block);

        return 0;
    }

    virtual void GetStreamHeader( stream_header &h)
    {
        h = m_header;
    }

    virtual void Seek( sampleno /*ignore*/)
    {
        // nop
    }


};
#endif // CONSTANT_PRODUCER_H
