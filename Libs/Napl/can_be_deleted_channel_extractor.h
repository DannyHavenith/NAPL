template< typename monotype>
class channel_extractor{};

template< typename monotype>
class channel_extractor< StereoSample< monotype> > : public block_mutator, private block_owner
{
	unsigned short m_channel;

public:
	channel_extractor( unsigned short channel)
		m_channel( channel)
	{
	}

	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num)
	{
		// this is not quite thread-safe. This object keeps the requesting consumer in it's
		// state until the next call to RequestBlock
		//
		// each thread should have it's own mutator...

		// save our current consumer on the stack.
		// this at least makes sure that re-entrant calls reply to the right consumer.
		state_saver<block_consumer *> save( m_pConsumer);
		m_pConsumer = &c;
		sampleno sub_block = 0;
		sampleno offset = 0;

		//
		// ask for at most one block full of samples until we have satisfied the original
		// request
		//
		while (num)
		{
			sub_block = std::min(  num, sampleno( m_block.m_size/sizeof monotype));
			m_pProducer->RequestBlock( *this, start + offset, sub_block);
			num -= sub_block;
			offset += sub_block;
		}



		return 0;
	};

	virtual void GetStreamHeader( stream_header &h) 
	{
		h = m_header;
		h.numchannels = 1;
	}

	virtual void Seek( sampleno start)
	{
		m_pProducer->Seek(start);
	}

	virtual void ReceiveBlock( const sample_block &b)
	{

	};
};