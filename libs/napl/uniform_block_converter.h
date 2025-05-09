#include "samplebl.h"
template<typename type>
struct size_code_converter
{
    static int get_size()
    {
        return 8 * sizeof( type);
    }
};

template<>
struct size_code_converter<double>
    {
        static int get_size()
        {
            return -2;
        }
    };

template <>
struct size_code_converter<float>
{
    static int get_size()
    {
        return -1;
    }
};

template <typename derived_type, typename source_type, typename destination_type>
struct typed_converter
{
    // the only method that the derived_type needs to implement is the
    // do_convert( source, destination)-method.
    // all the others may be overridden.
    //
    typedef void init_type;

    sampleno destination_type_samples( size_t bytes)
    {
        return bytes/sizeof( destination_type);
    }

    void advance_source( unsigned char *&source_ptr)
    {
        source_ptr += sizeof( source_type);
    }

    void advance_destination( unsigned char *&dest_ptr)
    {
        dest_ptr += sizeof( destination_type);
    }

    derived_type &get_derived()
    {
        return *static_cast<derived_type *>(this);
    }

    void convert( unsigned char *source_ptr, unsigned char *dest_ptr)
    {
        get_derived().do_convert(
            reinterpret_cast<source_type *>( source_ptr),
            reinterpret_cast<destination_type *>(dest_ptr));
    }


    void MutateHeader( stream_header &h)
    {
        h.numchannels = sampletraits< destination_type>::get_num_channels();
        typedef typename sampletraits< destination_type>::channel_type channel_type;


        h.samplesize = size_code_converter<channel_type>::get_size();
    }
};


//
// adapter for mutators that originally work inline
// this adapter ensures that these inline mutators can also be used in converters
// that copy data from one block to another.
//
template< typename mutator>
struct mutator_adapter :
    public typed_converter< mutator_adapter<mutator>, typename mutator::sample_type, typename mutator::sample_type>,
    public mutator
{
    typedef typename mutator::sample_type sample_type;

    void do_convert( sample_type *source, sample_type *destination)
    {
        sample_type temp = *source;
        mutator::Mutate( &temp);
        *destination = temp;
    }

    void MutateHeader( stream_header &h)
    {
        mutator::MutateHeader( h);
    }
};

// analog to the uniform block mutator, this component acts as a sample-by-sample converter
// the main difference is that the uniform block mutator works inline (in the block itself)
// where the converter works from one block to another.
template < typename converter_type, typename interface_type = block_mutator>
class uniform_block_converter: public interface_type, private block_owner
{
protected:
    converter_type m_converter;

public:

    template< typename converter_init_type>
    explicit uniform_block_converter( converter_init_type i)
    : m_converter( i)
    {
    }

    uniform_block_converter( ) = default;


    void GetStreamHeader( stream_header &h) override
    {
        interface_type::m_pProducer->GetStreamHeader( h);
        m_converter.MutateHeader( h);
    }

    sample_block RequestBlock( sampleno start, unsigned long num) override
    {
        const auto source_block = interface_type::m_pProducer->RequestBlock( start, num);

        auto  destination_block( get_block());
        destination_block.Truncate( source_block.size());

        sampleno frame_count = m_converter.destination_type_samples( destination_block.size());

        destination_block.m_start = destination_block.buffer_begin();

        unsigned char *pSrcFrame = source_block.m_start;
        unsigned char *pDstFrame = destination_block.m_start;

        while (pDstFrame < destination_block.m_end)
        {
            // convert the source frame to the target frame
            // and advance the pointers to the next frame.
            m_converter.convert( pSrcFrame, pDstFrame);
            m_converter.advance_source( pSrcFrame);
            m_converter.advance_destination( pDstFrame);
        }

        destination_block.m_end = pDstFrame;
        return destination_block;
    }

private:

};
