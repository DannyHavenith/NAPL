#include "stdafx.h"
#include "samplebl.h"

struct memory_writer_base: public block_consumer
{
	memory_writer_base( block_producer *p, void *destination)
		: m_destination( reinterpret_cast< sample_block::byte_type *>(
									destination))
	{
		LinkTo( p);
	}

	void *Write( sampleno start, sampleno frames)
	{
		m_pProducer->RequestBlock( *this, start, frames);
		return m_destination;
	}


protected:
	sample_block::byte_type *m_destination;
};

struct memory_writer : public memory_writer_base
{
	memory_writer( block_producer *p, void *destination)
		: memory_writer_base( p, destination)
	{
	}

	virtual void ReceiveBlock( const sample_block &b)
	{
		m_destination =
			std::copy( b.m_start, b.m_end, m_destination);
	}
};

template< typename sample_type, typename binary_op = std::plus< sample_type> >
struct memory_adder : public memory_writer_base
{

	memory_adder( block_producer *p, sample_type *destination)
		: memory_writer_base( p, destination)
	{
	}

	virtual void ReceiveBlock( const sample_block &b)
	{
		const sample_container< sample_type> container( b);
		m_destination =reinterpret_cast< sample_block::byte_type *>(
				std::transform(
					container.begin(),
					container.end(),
					reinterpret_cast< sample_type *>( m_destination),
					reinterpret_cast< sample_type *>( m_destination),
					binary_op()
					)
			);

	}
};

void *write_frames_to_memory( block_producer *p,
							 void *memory,
							 sampleno start,
							 sampleno samples
							 )
{
	memory_writer w( p, memory);
	return w.Write( start, samples);
}

void *add_floats_to_memory( block_producer *p,
						   float *memory,
						   sampleno start,
						   sampleno samples
						   )
{
	memory_adder<float> adder( p, memory);
	return adder.Write( start, samples);
}
