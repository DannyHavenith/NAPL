/**
 * \ingroup Napl
 * \brief Sink that does nothing
 *
 * This sink just requests all samples from it's producer and does nothing
 * with them.
 * This is convenient for graphs that do not have to write to an external device
 * and that may contain an analyzer, for instance.
 *
 * \version 1.0
 * first version
 *
 * \date 11-25-2005
 *
 * \author Danny
 *
 *
 */
#if !defined( NIL_SINK_H)
#define NIL_SINK_H

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

	/**
	 * This implementation of ReceiveBlock discards the received samples.
	 */
	virtual void ReceiveBlock( const sample_block &)
	{
		// nop
	};
};
#endif //NIL_SINK_H
