//
// given a template, instantiate this template for all known template types and 
// provide a mapping from stream-headers to the instantiated template.
//
template< template< typename T> class to_instantiate, typename interface_type>
struct sample_factory_instantiator
{
	static interface_type *instantiate( const stream_header &h)
	{
		if ( 2 == h.numchannels)
		{
			// stereo samples
			if ( 16 == h.samplesize)
			{
				return new to_instantiate< sampletype_s16>();
			}
			else if ( 8 == h.samplesize)
			{
				return new to_instantiate< sampletype_s8>();
			}
			else if (24 == h.samplesize)
			{
				return new to_instantiate< sampletype_s24>();
			}
			else if (-2 == h.samplesize)
			{
				return new to_instantiate< sampletype_sd>();
			}
			else return 0;
		}
		else if ( 1 == h.numchannels)
		{
			if ( 16 == h.samplesize)
			{
				return new to_instantiate< sampletype_m16>();
			}
			else if ( 8 == h.samplesize)
			{
				return new to_instantiate< sampletype_m8>();
			}
			else if (24 == h.samplesize)
			{
				return new to_instantiate< sampletype_m24>();
			}
			else if (-2 == h.samplesize)
			{
				return new to_instantiate< sampletype_md>();
			}
			else return 0;
		}
		else if ( 4 == h.numchannels)
		{
			if ( 16 == h.samplesize)
			{
				return new to_instantiate< sampletype_q16>();
			}
			else if ( 8 == h.samplesize)
			{
				return new to_instantiate< sampletype_q8>();
			}
			else if (24 == h.samplesize)
			{
				return new to_instantiate< sampletype_q24>();
			}
			else if (-2 == h.samplesize)
			{
				return new to_instantiate< sampletype_qd>();
			}
			else return 0;
		}

		else return 0;

	}
};