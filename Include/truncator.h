
class truncator: public block_mutator, private block_owner
{
public:

	truncator( unsigned short samplesize)
		: m_new_samplesize( samplesize)
		
	{
		
	}

	virtual void Seek( sampleno start)
	{
		m_pProducer->Seek( start);
	}

	virtual void GetStreamHeader( stream_header &h)
	{
		m_pProducer->GetStreamHeader( h);

		m_SrcFrameSize = h.get_sample_size(); // in bytes



		if ((h.architecture & SH_ARCH_ENDIAN) != ARCHITECTURE_LITTLEENDIAN)
		{
			m_skip_front = 0;
			m_skip_back = h.get_sample_size()/8 - m_new_samplesize;
		}
		else
		{
			m_skip_back = 0;
			m_skip_front = h.get_sample_size()/8 - m_new_samplesize;
		}
		h.samplesize = m_new_samplesize * 8; // in bits

	}

	virtual inline block_result RequestBlock( block_consumer &c, sampleno start, unsigned long num)
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
			sub_block = std::min(  num, sampleno( m_block.buffer_size()/m_new_samplesize));
			m_pProducer->RequestBlock( *this, start + offset, sub_block);
			num -= sub_block;
			offset += sub_block;
		}



		return 0;
	}


	virtual void ReceiveBlock( const sample_block &b)
	{
		m_block.m_start = m_block.buffer_begin();
		unsigned char *pSrcFrame = b.m_start;
		unsigned char *pTgtFrame = m_block.m_start;

		while (pSrcFrame < b.m_end)
		{
			pSrcFrame+= m_skip_front;

			// copy the significant bytes
			for (short counter = m_new_samplesize; counter; --counter)
			{
				*pTgtFrame++ = *pSrcFrame++;
			}

			pSrcFrame += m_skip_back;
		}

		m_block.m_end = pTgtFrame;
		m_pConsumer->ReceiveBlock( m_block);

	}

private:

	short m_skip_front;
	short m_skip_back;
	short m_SrcFrameSize;

	unsigned short m_new_samplesize;
};
