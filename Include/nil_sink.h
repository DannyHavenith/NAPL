struct nil_sink : public block_sink
{
	virtual void Start()
	{
		if (m_pProducer)
		{
			stream_header h;
			m_pProducer->GetStreamHeader( h);
			m_pProducer->RequestBlock( *this, 0, h.numframes);
		};
	}

	virtual void ReceiveBlock( const sample_block &b) 
	{
		// nop
	};
};