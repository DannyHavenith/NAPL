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

struct buffer_allocator
{
private:
	long m_buffer_count;
	buffer_allocator() : m_buffer_count(0){};
	~buffer_allocator()
	{
		std::cerr << "allocated " << m_buffer_count << " buffers" << std::endl;
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

////////////////////////////////////////////////////////////////////////////////
//
// Sample_block contains some contiguous memory with samples
//
// sample_blocks get shifted around from one processor to the other.
//
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

//
// this class acts as a wrapper to make sample blocks behave more like 
// a standard container class
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

////////////////////////////////////////////////////////////////////////////////
//
// block_owner is a base type that represents objects that have their own block
// and fill it with sample data
//
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

////////////////////////////////////////////////////////////////////////////////
// 
// stream_header gives information about the sample streams that flow from one
// processor to the other.
//
struct stream_header
{
	unsigned long samplerate; // frames per second
	short samplesize; // bits per sample, negative values have special meanings.
	unsigned short numchannels; // samples per frame
	unsigned long numframes;
	unsigned short architecture; // endian-nes and interleaving of samples

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

////////////////////////////////////////////////////////////////////////////////
//
// A block_producer is an object that can produce sample_blocks on demand
// processors typically expose a block_producer side and a block_consumer side
// the consumer side is exposed to producers and vice versa.
//
class block_producer
{
public:
	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num) = 0;
	virtual void GetStreamHeader( stream_header &h) = 0;

	virtual void Seek( sampleno start) = 0;

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


////////////////////////////////////////////////////////////////////////////////
//
// block_consumers receive blocks with sample data from block_producers.
// They can be linked to producers.
//
class block_consumer
{
public:
	block_consumer()
	{};

	block_consumer( block_producer *p)
	{
		LinkTo( p);
	}

	virtual void LinkTo( block_producer *p)
	{
		m_pProducer = CheckArchitecture( p);
		NotifyProducer();
	}

	virtual void ReceiveBlock( const sample_block &b) = 0;

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

////////////////////////////////////////////////////////////////////////////////
//
// A block_sink is a consumer that does not have a producer side. A sample 
// stream ends here.
//
// subclasses override the 'Start' function to start the whole chain
//
class block_sink : public block_consumer
{
public:
	virtual void Start() = 0;
};


////////////////////////////////////////////////////////////////////////////////
//
// A block_mutator is an object that exposes both a consumer and a producer side
//
class block_mutator: public block_consumer, public block_producer
{
};



////////////////////////////////////////////////////////////////////////////////
//
// creative_block_producers have their own block. They fill it and send it to
// the consumer.
//
// subclasses override the 'FillBlock' function to provide real functionality
//
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
	// this function is called whenever a block of samples is requested
	virtual sampleno FillBlock( sample_block &b, sampleno count) = 0;

	// this function is called once for every newly created block
	virtual void InitBlock( sample_block &b) {};

	block_result m_result;
};

//
// helper class that can save the state of some member during
// a function call and restore that state after the function call
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

////////////////////////////////////////////////////////////////////////////////
//
// uniform_block_mutators perform some operation on each sample in the sample stream.
// the operation and the sample type is determined by the sample_mutator parameter.
//
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

////////////////////////////////////////////////////////////////////////////////
//
// block_multi_consumers are consumers that have multiple inputs.
//
// They need block_connectors to operate properly.
//
class block_multi_consumer
{
public:
	virtual void ReceiveBlock( const sample_block &b, short channel) = 0;

protected:
	// 
	// The GetMcPtr() - function enables derived classes to use a pointer to 
	// 'this' in their base member initialisation.
	inline block_multi_consumer *GetMcPtr() { return this;}
};

////////////////////////////////////////////////////////////////////////////////
//
// block_connectors provide a standard block_consumer interface for each input
// of a block_multi_consumer.
//
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

////////////////////////////////////////////////////////////////////////////////
//
// A binary_block_mutator is a binary_block_processor that delegates real 
// processing to a 'mutator' object that is given as a template parameter.
//
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




#endif
