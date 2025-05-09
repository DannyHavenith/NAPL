#if !defined( NIL_SINK_H)
#define NIL_SINK_H
#include "samplebl.h"

/**
 *  Sink that does nothing
 *
 * This sink just requests all samples from it's producer and does nothing
 * with them.
 * This is convenient for graphs that do not have to write to an external device
 * and that may contain an analyzer, for instance.
 *
 */
struct nil_sink : public block_sink
{
    void Start() override
    {
        FetchAll();
    }

	/**
	 * This implementation of ReceiveBlock discards the received samples.
	 */
	void ReceiveBlock( const sample_block &) override
	{
		// nop
	};
};
#endif //NIL_SINK_H
