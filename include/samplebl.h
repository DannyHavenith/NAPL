//
// sampleblock.h - define sample blocks, producers and consumers
//

#ifndef _SAMPLEBLOCK_H_
#define _SAMPLEBLOCK_H_
#include "architec.h"

#include <algorithm>
#include <boost/core/noncopyable.hpp>
#include <cassert>
#include <memory>
#include <stddef.h>
#include <vector>

#define BLOCK_ERROR 1
typedef int block_result;
typedef unsigned long sampleno;

class block_consumer;
class block_producer;


/**
 * \ingroup Napl
 * Simple buffer that consists of a pointer to a block of bytes and
 * a size.
 *
 * \version 1.0
 * first version
 *
 * \date 12-21-2004
 *
 * \author Danny
 *
 *
 */
struct byte_buffer
{
    typedef unsigned char byte_type;

    byte_buffer( size_t size)
        : m_ptr( new byte_type[size]),
        m_size( size),
        m_is_owner( true)
    {
    }

    byte_buffer( byte_type *ptr, size_t size)
        : m_ptr( ptr),
        m_size( size),
        m_is_owner( false)
    {
    }

    byte_buffer()
        : m_ptr(0), m_size(0)
    {
        // nop
    }

    ~byte_buffer()
    {
        if (m_is_owner)
        {
            delete [] m_ptr;
        }
    }

    byte_type *get_begin()
    {
        return m_ptr;
    }

    size_t get_size() const
    {
        return m_size;
    }

    byte_type *m_ptr;
    size_t m_size;
    bool m_is_owner;
};

/**
 * \ingroup Napl
 * Buffer allocation singleton.
 * The current implementations will simply create new file buffers.
 *
 * \version 1.0
 * first version
 *
 * \date 12-21-2004
 *
 * \author Danny
 *
 *
 */
struct buffer_allocator
{
private:
    long m_buffer_count;
    buffer_allocator() : m_buffer_count(0){};
    ~buffer_allocator()
    {
    }

public:
    static buffer_allocator &get_instance()
    {
        static buffer_allocator the_instance;
        return the_instance;
    }

    byte_buffer *create_buffer( size_t size)
    {
        ++m_buffer_count;
        return new byte_buffer( size);
    }


};

/**
 * \ingroup Napl
 * A sample block acts as a 'shared pointer' to a byte buffer.
 * Additionally a sample block may define a 'window' on the byte buffer,
 * allowing clients to only access part of the buffer.
 *
 * \version 1.0
 * first version
 *
 * \date 12-21-2004
 *
 * \author Danny
 *
 *
 */
struct sample_block
{
    typedef byte_buffer::byte_type byte_type;
    typedef std::shared_ptr< byte_buffer> buffer_ptr_type;


    sample_block(){};
    sample_block( buffer_ptr_type buffer)
    :    m_buffer_ptr( std::move( buffer))
    {
        Reset();
    }

    sample_block( size_t size)
    {
        Init( buffer_ptr_type( buffer_allocator::get_instance().create_buffer( size)));
    }

    void Init( buffer_ptr_type buffer)
    {
        m_buffer_ptr =  std::move( buffer);
        Reset();
    }

    /**
     * Ensure that the size of the block is at most the given size.
     *
     * The block size could be smaller if it was smaller to begin with.
     */
    void Truncate( size_t size)
    {
        m_end = m_start + std::min( size, m_buffer_ptr->get_size());
    }

    /**
     * return the amount of bytes in the block.
     *
     */
    std::size_t size() const
    {
        return m_end - m_start;
    }

    void Reset()
    {
        m_start = m_buffer_ptr->get_begin();
        m_end = m_start + m_buffer_ptr->get_size();
        std::fill( m_start, m_end, 0);
    }

    byte_type *buffer_begin()
    {
        return m_buffer_ptr->get_begin();
    }

    size_t buffer_size() const
    {
        return m_buffer_ptr->get_size();
    }

    // the members that are used by consumers
    // this defines a 'window' on the samples that is
    // actually seen by consumers.
    byte_type *m_start;
    byte_type *m_end;

    //
    // The shared pointer to the actual buffer bytes
    //
    buffer_ptr_type m_buffer_ptr;
};

/**
 * \ingroup Napl
 * block managers create blocks of specific sizes while cacheing returned blocks
 * for later re-use
 *
 * \version 1.0
 * first version
 *
 * \date 12-21-2004
 *
 * \author Danny
 *
 *
 */
class block_manager
{
public:


    block_manager( size_t size)
        : m_block_size( size)
    {
        // nop
    }

    //
    // gets a new or re-used block
    // returns false if the block is new (un-initialized) and true
    // if it is a re-used block.
    //
    sample_block get_block()
    {
        std::shared_ptr<byte_buffer> buffer;
        if (m_free_buffers.size())
        {
            buffer.swap( m_free_buffers.back());
            m_free_buffers.pop_back();
        }
        else
        {
            buffer = std::make_shared< byte_buffer>( m_block_size);
        }

        return {buffer};
    }

    size_t get_block_size() const
    {
        return m_block_size;
    }

private:
    using buffer_list_t = std::vector< std::shared_ptr<byte_buffer>>;
    buffer_list_t m_free_buffers;
    size_t m_block_size;
};



/**
 * \ingroup Napl
 * this template acts as a wrapper to make sample blocks behave more like
 * a standard container class
 *
 * \version 1.0
 * first version
 *
 * \date 12-21-2004
 *
 * \author Danny
 *
 *
 */
template <class sample_type>
struct sample_container
{
private:
    const sample_block &m_block;
public:
    typedef sample_type *iterator;
    typedef const sample_type *const_iterator;

    sample_container( const sample_block &block)
        :m_block( block)
    {
        // nop
    ;
    }

    iterator begin()
    {
        return (iterator) m_block.m_start;
    }

    const_iterator begin() const
    {
        return (const_iterator)m_block.m_start;
    }

    iterator end()
    {
        return begin() + ((m_block.m_end - m_block.m_start)/sizeof( sample_type));
    }

    const_iterator end() const
    {
        return begin() + ((m_block.m_end - m_block.m_start)/sizeof( sample_type));
    }

    std::size_t size() const
    {
        return end() - begin();
    }
};

/**
 * \ingroup Napl
 * block_owner is a base type that represents objects that have their own block
 * and fill it with sample data
 *
 * \version 1.0
 * first version
 *
 * \date 12-22-2004
 *
 * \author Danny
 *
 *
 */
class block_owner : protected block_manager
{
public:
    block_owner()
        : block_manager( 1024 * 1024)
    {

    }

    explicit block_owner( size_t size)
        :block_manager( size)
    {

    }

};

/**
 * \ingroup Napl
 * stream_header gives information about the sample streams that flow from one
 * processor to the other.
 *
 * \version 1.0
 * first version
 *
 * \date 12-22-2004
 *
 * \author Danny
 *
 *
 */
struct stream_header
{

    stream_header(
        unsigned long rate,
        short size,
        unsigned short channels,
        unsigned long frames,
        unsigned short arch
        )
        : samplerate( rate),
        samplesize( size),
        numchannels( channels),
        numframes( frames),
        architecture( arch)
    {
        // nop
    }

    stream_header()
    {
        // nop
    }

    unsigned long samplerate; ///< frames per second
    short samplesize; ///< bits per sample, negative values have special meanings.
    unsigned short numchannels; ///< samples per frame
    unsigned long numframes;
    unsigned short architecture; ///< endian-nes and interleaving of samples

    //
    // frame size in bytes
    inline unsigned short frame_size() const
    {
        return (get_sample_size()/8) * numchannels;
    }

    inline unsigned short get_sample_size() const
    {
        if (samplesize == -2) return 8 * sizeof( double);
        else
        {
            return samplesize;
        }
    }
};


/**
 * \ingroup Napl
 * A block_producer is an object that can produce sample_blocks on demand
 * processors typically expose a block_producer side and a block_consumer side
 * the consumer side is exposed to producers and vice versa.
 *
 * \version 1.0
 * first version
 *
 * \date 12-22-2004
 *
 * \author Danny
 *
 *
 */
class block_producer
{
public:
    /**
     * Request samples from the producer.
     *
     * The producer will return a block that
     * contains at least the sample at index 'start' and that may contain less
     * than 'num' samples, but not more. It is up to the caller to repeatedly
     * call this function until all samples are received.
     */
    virtual sample_block RequestBlock( sampleno start, unsigned long num) = 0;

    /**
     * Get the stream metadata, such as samplerate, samplesize, etc.
     * \param &h [out] The header to fill with metadata for this producer. See also stream_header
     */
    virtual void GetStreamHeader( stream_header &h) = 0;

    virtual ~block_producer() = default;

};

using block_producer_ptr = std::shared_ptr<block_producer>;


// forward declaration of a function that is defined in objfact.cpp
extern block_producer_ptr GlobalGetEndianConverter( const stream_header &h, block_producer_ptr pP);


/**
 * \ingroup Napl
 * block_consumers receive blocks with sample data from block_producers.
 * They can be linked to producers.
 *
 * \version 1.0
 * first version
 *
 * \date 12-22-2004
 *
 * \author Danny
 *
 *
 */
class block_consumer
{
public:

    /**
     * normal c'tor, nothing fancy
     */
    block_consumer()
    {};

    virtual ~block_consumer() = default;

    /**
     * construct and link to a producer
     * \param *p [in] producer to link to
     */
    block_consumer( block_producer_ptr p)
    {
        LinkTo( std::move( p));
    }

    /**
     * Explicitly link to a producer.
     * The default implementation will also check the architecture
     * of the producer and add an endian-converter if needed.
     * \param *p [in] producer to link to
     */
    virtual void LinkTo( block_producer_ptr p)
    {
        m_pProducer = CheckArchitecture( std::move(p));
    }



protected:
    //
    // the architecture of this consumer is the architecture of the local machine
    //
    virtual unsigned long GetArchitecture() { return LOCAL_ARCHITECTURE;}

    /**
     * Check a producers architecture (specifically: endiannes) and
     * create a endian-converter if needed.
     * \param *pP [in] the producer to check
     * \return eiter the original producer pointer, or a pointer to an endian-converter that
     * has been linked to the producer.
     */
    virtual block_producer_ptr CheckArchitecture( block_producer_ptr pP)
    {
        stream_header h;
        if (pP)
        {
            pP->GetStreamHeader( h);
            if ( (h.architecture ^ GetArchitecture()) & SH_ARCH_ENDIAN)
            {
                // endiannes differs between this consumer and the producer
                // use an endian-converter
                //
                return ::GlobalGetEndianConverter( h, pP);
            }
            else return pP;
        }
        return pP;

    }

    block_producer &GetProducer()
    {
        return *m_pProducer;
    }

    block_producer_ptr m_pProducer;

};

/**
 * \ingroup Napl
 * A block_sink is a consumer that does not have a producer side. A sample
 * stream ends here.
 *
 * subclasses override the 'Start' function to start the whole chain
 *
 * \version 1.0
 * first version
 *
 * \date 12-22-2004
 *
 * \author Danny
 *
 *
 */
class block_sink : public block_consumer
{
public:

    virtual void Start() = 0;

protected:

    /**
     * Start retrieving data from a producer.
     */
     void FetchAll()
     {
         if (m_pProducer)
         {
             stream_header h;
             m_pProducer->GetStreamHeader( h);

             sampleno framesLeft = h.numframes;
             sampleno currentOffset = 0;
             while (framesLeft > 0)
             {
                 const auto block = m_pProducer->RequestBlock( currentOffset, framesLeft);
                 ReceiveBlock( block);
                 const auto frame_count = SampleCount( block, h);
                 currentOffset += frame_count;
                 framesLeft -= frame_count;
             }
         }
     }

     virtual void ReceiveBlock( const sample_block &b) = 0;

private:
    static sampleno SampleCount( const sample_block &b, const stream_header &h)
    {
        return (b.size()) / h.frame_size();
    }
};


/**
 * \ingroup Napl
 *  A block_mutator is an object that exposes both a consumer and a producer side
 *
 * \version 1.0
 * first version
 *
 * \date 12-22-2004
 *
 * \author Danny
 *
 *
 */
class block_mutator: public block_consumer, public block_producer
{
};


/**
 * \ingroup Napl
 *
 * creative_block_producers have their own block. They fill it and send it to
 * the consumer.
 *
 * subclasses override the 'FillBlock' function to provide real functionality
 *
 * \version 1.0
 * first version
 *
 * \date 12-22-2004
 *
 * \author Danny
 *
 *
 */
class creative_block_producer : public block_producer, protected block_owner
{
public:

    creative_block_producer()
    {
        // nop
    }

    creative_block_producer( size_t block_size)
        : block_owner( block_size)
    {
        // nop
    }


    /**
    * Request a block of data.
    * This implementation of the RequestBlock method will call the 'FillBlock' virtual function,
    * which enables derived classes to fill the blocks with actual sample data.
     * \param &consumer
     * \param start
     * \param num
     * \return
     */
     sample_block RequestBlock( sampleno start, unsigned long num) override
     {

         unsigned long nReceived;

         Seek( start);

         sample_block block{ get_block()};
         InitBlock( block);

         nReceived = FillBlock( block, num);
        return block;
    }

protected:

        /**
        * Fill a block with new sample data.
        * Implement this virtual method in a concrete creative block producer.
        *
        * \param &b [out] block to fill with data
        * \param count requested sampledata
        * \return
        */
        virtual sampleno FillBlock( sample_block &b, sampleno count) = 0;

        virtual void Seek( sampleno start) = 0;

    /**
     * Initialize a newly created block.
     * This method is called once for each block that is newly created. Implement this method
     * if you want to perform a specific action to initialize newly created blocks. An example of
     * such an initialization-action could be filling the block with zeros.
     * \param &b [in,out] the block to be initialized.
     */
    virtual void InitBlock( sample_block &b) {};

    block_result m_result;
};


/**
 * \ingroup Napl
 * uniform_block_mutators perform some operation on each sample in the sample stream.
 * the operation and the sample type is determined by the sample_mutator parameter.
 *
 * \version 1.0
 * first version
 *
 * \date 12-22-2004
 *
 * \author Danny
 *
 *
 */
template< class sample_mutator, class interface_type = block_mutator>
class uniform_block_mutator: public interface_type, private block_owner
{
public:

    // just relay the header request to our producer.
    virtual void GetStreamHeader( stream_header &h)
    {
        interface_type::m_pProducer->GetStreamHeader( h);
        m_sample_mutator.MutateHeader( h);
    }

    /**
     * This implementation delegates the block request to the linked producer.
     * \see block_producer::RequestBlock( block_consumer &c, sampleno start, unsigned long num)
     * \param &c
     * \param start
     * \param num
     * \return
     */
    sample_block RequestBlock( sampleno start, unsigned long num) override
    {
        using sample_type = typename sample_mutator::sample_type;

        auto destination_block = get_block();
        sample_container<sample_type> destination{ destination_block};

        const auto source_block = interface_type::m_pProducer->RequestBlock( start, std::min( std::size( destination), num));
        const sample_container< sample_type> source{ source_block};
        destination_block.Truncate( source_block.size());

        std::transform( source.begin(), source.end(), destination.begin(), m_sample_mutator);

        return destination_block;
    }

protected:
    sample_mutator m_sample_mutator;
};



/**
 * \ingroup Napl
 * \brief a consumer of more than one producer.
 * block_multi_consumers are consumers that have multiple inputs.
 * They need more than one block_connector to operate properly.
 *
 * \version 1.0
 * first version
 *
 * \date 12-23-2004
 *
 * \author Danny
 *
 *
 */

////////////////////////////////////////////////////////////////////////////////
//
// A binary_block_processor is a special kind of multi_consumer, having 2 inputs
//
/**
 * \ingroup Napl
 * a binary_block_processor is a special case of a block_multi_consumer that has
 * exactly 2 inputs
 * \version 1.0
 * first version
 *
 * \date 12-24-2004
 *
 * \author Danny
 *
 *
 */
class binary_block_processor: public block_producer
{
public:
    void GetStreamHeader( stream_header &h) override
    {

        stream_header h1;
        stream_header h2;
        m_left->GetStreamHeader( h1);
        m_right->GetStreamHeader( h2);

        h = h2;
        if (h1.numframes < h2.numframes) h.numframes = h1.numframes;

        m_leftFrameSize = h1.frame_size();
        m_rightFrameSize = h2.frame_size();

        MutateHeader( h, h1, h2);

    }

    virtual void MutateHeader( stream_header &result, const stream_header &left, const stream_header &right) = 0;

    /**
     * a binary block processor will first send the block request to it's left producer. When it
     * receives a block from the left producer, it will ask for the corresponding block of its right
     * producer.
     * \param &consumer
     * \param start
     * \param num
     * \return
     */
    sample_block RequestBlock( sampleno start, unsigned long num) override
    {
        assert( m_left && m_right);

        auto block_left = m_left->RequestBlock( start, num);
        const std::size_t left_frame_count = block_left.size() / m_leftFrameSize;
        auto block_right = m_right->RequestBlock( start, std::min( num, left_frame_count));

        // make sure that the left block is the same size as the right block in the number of frames
        block_left.Truncate( (block_right.size() / m_rightFrameSize) * m_leftFrameSize);

        return ProcessBlocks( block_left, block_right);

    }

    virtual void LinkTo( block_producer_ptr pLeft, block_producer_ptr pRight)
    {
        m_left = std::move( pLeft);
        m_right = std::move( pRight);
    }


protected:
    /**
     * Process the left and right input blocks and return the result in a new block.
     *
     * The left and right blocks are guaranteed to contain the same number of frames.
     */
    virtual sample_block ProcessBlocks( const sample_block &left, const sample_block &right) = 0;

    unsigned short m_leftFrameSize{1};
    unsigned short m_rightFrameSize{1};
    block_producer_ptr m_left;
    block_producer_ptr m_right;
};

/**
 * \ingroup Napl
 * A binary_block_mutator is a binary_block_processor that delegates real
 * processing to a 'mutator' objct that is given as a template parameter.
 * \version 1.0
 * first version
 *
 * \date 12-25-2004
 *
 * \author Danny
 *
 *
 */
template <typename mutator>
class asymmetric_binary_block_mutator : public binary_block_processor, private block_owner
{
public:
    typedef typename mutator::sample_type sample_type;
    typedef typename mutator::left_type left_type;

protected:
    sample_block ProcessBlocks( const sample_block &left, const sample_block &right) override
    {

        auto result = get_block();
        const auto samples = std::min( left.size() / sizeof( left_type), right.size() / sizeof( sample_type));
        result.Truncate( samples * sizeof( sample_type));

        sample_container< sample_type> result_samples{ result};


        left_type *pLeft = reinterpret_cast< left_type *>(left.m_start);
        sample_type *pRight = reinterpret_cast< sample_type *>(right.m_start);

        for ( auto resultValue = result_samples.begin(); resultValue != result_samples.end();)
        {
            *resultValue = m_mutator.Mutate( *pLeft, *pRight);
            ++resultValue;
            ++pRight;
            ++pLeft;
        }

        return result;
    }

    virtual void MutateHeader( stream_header &result, const stream_header &left, const stream_header &right)
    {
        m_mutator.MutateHeader( result, left, right);
    }

    mutator m_mutator;
};

template< typename symmetric_mutator>
struct symmetric_mutator_adapter : public symmetric_mutator
{
    typedef typename symmetric_mutator::sample_type left_type;
};

template< typename symmetric_mutator>
struct binary_block_mutator :
    public asymmetric_binary_block_mutator< symmetric_mutator_adapter< symmetric_mutator> >
{

};

////////////////////////////////////////////////////////////////////////////////
//
// mut_add is a typical mutator that uses the binary_block_mutator to add two
// sample streams together.
//
template< typename T> struct sampletraits;
template <typename sampletype>
class mut_add
{
public:
    typedef sampletype sample_type;

    static inline void MutateHeader( stream_header &, const stream_header &, const stream_header &)
    {
        // this mutator does not alter the header
    }

    static inline const sampletype Mutate(
        const sampletype &left, const sampletype &right)
    {
        using accumulatortype =  typename sampletraits<sampletype>::template accumulator_gen<void>::type;
        using channel_type = typename sampletraits<sampletype>::template channel_gen<void>::type;
        using channel_traits = sampletraits<channel_type>;
        using accumulator_traits = sampletraits<accumulatortype>;

        auto result = accumulatortype(left) + accumulatortype(right);
        const auto clamp = []( auto &sample)
        {
            using accumulator_sampletype = typename std::remove_reference<decltype( sample)>::type;
            sample = std::clamp(
                sample,
                static_cast<accumulator_sampletype>(channel_traits::get_min()), static_cast<accumulator_sampletype>(channel_traits::get_max()));
        };
        accumulator_traits::apply_to_all_channels(clamp, result);

        return result;
    }
};

template<>
class mut_add<double>
{
public:
    typedef double sample_type;

    static inline void MutateHeader( stream_header &, const stream_header &, const stream_header &)
    {
        // this mutator does not alter the header
    }

    static inline const sample_type Mutate(
        const sample_type &left, const sample_type &right)
    {
        return left + right;
    }
};



#endif
