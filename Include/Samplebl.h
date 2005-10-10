//
// sampleblock.h - define sample blocks, producers and consumers
//

#ifndef _SAMPLEBLOCK_H_
#define _SAMPLEBLOCK_H_
#include <stddef.h>
#include "architec.h"
#include "boost/shared_ptr.hpp"
#include "boost/utility.hpp"

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
		m_size( size)
	{
	}

	byte_buffer()
		: m_ptr(0), m_size(0)
	{
		// nop
	}

	~byte_buffer()
	{
		delete [] m_ptr;
	}

	byte_type *get_begin()
	{
		return m_ptr;
	}

	size_t get_size()
	{
		return m_size;
	}

	byte_type *m_ptr;
	size_t m_size;
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


	inline sample_block(){};

	inline sample_block( size_t size)
	{
		Init( size);
	}

	inline void Init( size_t size)
	{
		m_buffer_ptr.reset( buffer_allocator::get_instance().create_buffer( size));
		m_start = m_buffer_ptr->get_begin();
		m_end = m_start + m_buffer_ptr->get_size();
	}


	byte_type *buffer_begin()
	{
		return m_buffer_ptr->get_begin();
	}

	size_t buffer_size()
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
	boost::shared_ptr< byte_buffer> m_buffer_ptr;
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
	typedef boost::shared_ptr<sample_block> block_ptr_t;
	typedef std::vector< block_ptr_t> block_list_t;

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
	bool get_block( block_ptr_t &result)
	{
		if (m_free_blocks.size())
		{
			result.swap( m_free_blocks.back());
			m_free_blocks.pop_back();
			return true;
		}
		else
		{
			result.reset( new sample_block( m_block_size));
			return false;
		}
	}

	void release_block( block_ptr_t &block)
	{
		m_free_blocks.push_back( block);
		block.reset();
	}

	size_t get_block_size() const
	{
		return m_block_size;
	}

private:
	block_list_t m_free_blocks;
	size_t m_block_size;

};


/**
 * \ingroup Napl
 * block handles obtain a block from a block manager and
 * return that block when their lifetime is over. 
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
class block_handle : public boost::noncopyable
{
	block_manager *m_manager_ptr;
	block_manager::block_ptr_t m_current_block;
	bool m_initialized;

public:
	block_handle( block_manager *manager)
		: m_manager_ptr( manager)
	{
		m_initialized = manager->get_block( m_current_block);
	}

	~block_handle()
	{
		m_manager_ptr->release_block( m_current_block);
	}

	sample_block &get_block()
	{
		return *m_current_block;
	}

	bool is_initialized()
	{
		return m_initialized;
	}
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

	const_iterator end() const
	{
		return begin() + ((m_block.m_end - m_block.m_start)/sizeof sample_type);
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
	 * Normally, a producer should react on this request by calling the consumers
	 * ReceiveBlock-method with the requested sample data.
	 * It is left to the producers discretion to divide the sample data over more than
	 * one block and call the consumers ReceiveBlock method consecutively.
	 * \param &consumer [in] The consumer in which to call ReceiveBlock
	 * \param start [in] zero-based index of first sample
	 * \param num  [in] number of samples to retreive.
	 * \return Either returns BLOCK_ERROR when something failed, or zero if successfull
	 */
	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num) = 0;

	/**
	 * Get the stream metadata, such as samplerate, samplesize, etc.
	 * \param &h [out] The header to fill with metadata for this producer. See also stream_header
	 */
	virtual void GetStreamHeader( stream_header &h) = 0;


	/**
	 * DEPRECATED: Seek to a sample offset.
	 * \param start offset
	 */
	virtual void Seek( sampleno start) = 0;

	/**
	 * Notify a producer that it should be expecting calls from this consumer.
	 * Please note that this is a hint only. One producer may have multiple consumers.
	 * Do not rely on the m_pConsumer member to point to the one and only consumer.
	 * \param *pC [in] The consumer to which this producer is linked.
	 */
	virtual void LinkToConsumer( block_consumer *pC)
	{
		m_pConsumer = pC;
	}

	virtual ~block_producer(){};

protected:
	block_consumer *m_pConsumer;
};


// forward declaration of a function that is defined in objfact.cpp
extern block_producer *GlobalGetEndianConverter( const stream_header &h, block_producer *pP);


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

	/**
	 * construct and link to a producer
	 * \param *p [in] producer to link to
	 */
	block_consumer( block_producer *p)
	{
		LinkTo( p);
	}

	/**
	 * Explicitly link to a producer.
	 * The default implementation will also check the architecture
	 * of the producer and add an endian-converter if needed.
	 * \param *p [in] producer to link to
	 */
	virtual void LinkTo( block_producer *p)
	{
		m_pProducer = CheckArchitecture( p);
		NotifyProducer();
	}

	/**
	 * Receive a block of sample data.
	 * Producers will react on a RequestBlock-call by calling the consumers
	 * ReceiveBlock a number of times with blocks of data.
	 * Implement this function for your specific producer implementation.
	 * \param &b [in] a block of data.
	 */
	virtual void ReceiveBlock( const sample_block &b) = 0;

	/**
	 * Notify a producer that it was linked to this consumer
	 */
	virtual void NotifyProducer()
	{
		if (m_pProducer)
			m_pProducer->LinkToConsumer( this);
	}

	virtual ~block_consumer(){};
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
	virtual block_producer *CheckArchitecture( block_producer *pP)
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
	block_producer *m_pProducer;

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
	/**
	 * Start retrieving data from a producer.
	 */
	virtual void Start() = 0;
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
	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num)
	{

		unsigned long nToGo = num;
		unsigned long nReceived;
		
		Seek( start);

		block_handle h( this); // releases the block on exit
		sample_block &block( h.get_block());
		if (!h.is_initialized())
		{
			InitBlock( block);
		}
		

		while ( nToGo && (nReceived = FillBlock( block, nToGo))) 
		{
			nToGo -= nReceived;
			consumer.ReceiveBlock( block);
		}

		if (!nToGo) return 0;
		else return BLOCK_ERROR;
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
 * helper class that can save the state of some member during
 * a function call and restore that state after the function call*  
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
template<typename T>
struct state_saver
{
	T value;
	T &reference;

	state_saver( T &origin_)
		: reference( origin_),
		value( origin_)
	{
		// nop
	}

	~state_saver()
	{
		reference = value;
	}
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
class uniform_block_mutator: public interface_type
{
public:

	virtual void Seek( sampleno start)
	{
		m_pProducer->Seek( start);
	}

	// just relay the header request to our producer.
	virtual void GetStreamHeader( stream_header &h)
	{
		m_pProducer->GetStreamHeader( h);
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
	virtual inline block_result RequestBlock( block_consumer &c, sampleno start, unsigned long num)
	{ 
		// this is not quite thread-safe. This object keeps the requesting consumer in it's
		// state until the next call to RequestBlock
		//
		// each thread should have it's own mutator...
		state_saver<block_consumer *> save( m_pConsumer);
		m_pConsumer = &c;

		// relay the block request
		return m_pProducer->RequestBlock( *this, start, num);
	}

	virtual void ReceiveBlock( const sample_block &b)
	{
		sample_mutator::sample_type *pSample = reinterpret_cast<sample_mutator::sample_type *>( b.m_start);

		// give our mutator the opportunity to change each sample
		while (pSample < reinterpret_cast<sample_mutator::sample_type *>( b.m_end))
			m_sample_mutator.Mutate( pSample++);

		// pass the changed block.
		m_pConsumer->ReceiveBlock( b);
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
class block_multi_consumer
{
public:
	/**
	 * The multi-consumer variant of the ReceiveBlock method has one 
	 * extra parameter: the 'channel'. This channel identifies the source
	 * of the data.
	 * \param &b the parameter block, see also block_consumer
	 * \param channel the channel, see also block_connector
	 */
	virtual void ReceiveBlock( const sample_block &b, short channel) = 0;

protected:
	// 
	// The GetMcPtr() - function enables derived classes to use a pointer to 
	// 'this' in their base member initialisation.
	inline block_multi_consumer *GetMcPtr() { return this;}
};

////////////////////////////////////////////////////////////////////////////////
//
//
//
/**
 * \ingroup Napl
 * block_connectors provide a standard block_consumer interface for each input
 * of a block_multi_consumer.
 *
 * \version 1.0
 * first version
 *
 * \date 12-24-2004
 *
 * \author Danny
 *
 *
 */
class block_connector: public block_consumer
{
public:
	block_connector( block_multi_consumer &parent, int channel)
		:m_parent( parent), m_channel( channel) {};

	virtual void ReceiveBlock( const sample_block &b)
	{
		m_parent.ReceiveBlock( b, m_channel);
	}

	block_result RequestBlock( sampleno start, unsigned long num)
	{
		return m_pProducer->RequestBlock( *this, start, num);
	}

	block_producer *GetProducer() { return m_pProducer;}

private:
	const short m_channel;
	block_multi_consumer &m_parent;
};

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
class binary_block_processor: public block_producer, public block_multi_consumer
{
public:

	binary_block_processor()
		: m_left( *GetMcPtr(), 0), m_right( *GetMcPtr(), 1) {;}

	virtual void GetStreamHeader( stream_header &h)
	{

		stream_header h1;
		stream_header h2;
		m_left.GetProducer()->GetStreamHeader( h1);
		m_right.GetProducer()->GetStreamHeader( h2);

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
	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num)
	{
		state_saver<block_consumer *> save( m_pConsumer);
		m_pConsumer = &consumer;

		m_currentLeft = start;
		return m_left.RequestBlock( start, num);
	}

	virtual void Seek( sampleno start)
	{
		m_currentLeft = start;
		m_left.GetProducer()->Seek( start);
	};

	/**
	 * If the block comes from the left-producer (channel 0), this function requests the corresponding 
	 * block from the right-producer. If the block comes from the right-producer (channel 1) both
	 * the left- and the right block will be offered to the ProcessBlocks virtual function.
	 * \param &b 
	 * \param channel 
	 */
	virtual void ReceiveBlock( const sample_block &b, short channel)
	{
		// if we receive a block from our zero channel, it's time to request the corresponding
		// data from our one channel.
		if (0 == channel)
		{
			m_leftBlock = b;
			if (b.m_end == b.m_start)
			{
				m_leftBlock = b;
			}
			sampleno sample_count = (sampleno)((b.m_end - b.m_start) / m_leftFrameSize);
			m_right.RequestBlock( 
				m_currentLeft, 
				sample_count);
			m_currentLeft += sample_count;
		}
		else
		{
			assert( (m_leftBlock.m_end - m_leftBlock.m_start) / m_leftFrameSize >= (b.m_end - b.m_start )/m_rightFrameSize);
//			m_leftBlock.m_end = m_leftBlock.m_start + (b.m_end - b.m_start);
			ProcessBlocks( m_pConsumer, m_leftBlock, b);
			m_leftBlock.m_start += ((b.m_end - b.m_start)/m_rightFrameSize) * m_leftFrameSize;
		}
	}
	
	virtual void LinkTo( block_producer *pLeft, block_producer *pRight)
	{
		m_left.LinkTo( pLeft);
		m_right.LinkTo( pRight);
	}


protected:
	virtual void ProcessBlocks( block_consumer *pC, const sample_block &left, const sample_block &right) = 0;

	sample_block m_leftBlock;
	unsigned short m_leftFrameSize;
	unsigned short m_rightFrameSize;
	sampleno m_currentLeft;
	block_connector m_left;
	block_connector m_right;
};


/**
 * \ingroup Napl
 * A binary_block_mutator is a binary_block_processor that delegates real 
 * processing to a 'mutator' object that is given as a template parameter.
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
class asymmetric_binary_block_mutator : public binary_block_processor
{
public:
	typedef typename mutator::sample_type sample_type;
	typedef typename mutator::left_type left_type;

protected:
	virtual void ProcessBlocks( block_consumer *pConsumer, const sample_block &left, const sample_block &right)
	{
		left_type *pLeft = reinterpret_cast< left_type *>(left.m_start);
		sample_type *pRight = reinterpret_cast< sample_type *>(right.m_start);
		sample_type *pEnd = reinterpret_cast< sample_type *>(right.m_end);

		while (pRight != pEnd)
		{
			*pRight = m_mutator.Mutate( *pLeft, *pRight);
			++pRight;
			++pLeft;
		}

		pConsumer->ReceiveBlock( right);
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
		typedef sampletraits<sampletype>::accumulator_gen<void>::type accumulatortype;
		return (sampletype)((accumulatortype(left) + accumulatortype(right)) /
						short(2));
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
