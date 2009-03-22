//
// given a template, instantiate this template for all known template types and
// provide a mapping from stream-headers to the instantiated template.
//
template< typename instantiate_metafunc, typename interface_type>
struct sample_factory_instantiator
{
	static interface_type *instantiate( const stream_header &h)
	{
		if ( 2 == h.numchannels)
		{
			// stereo samples
			if ( 16 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_s16>();
			}
			else if ( 8 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_s8>();
			}
			else if (24 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_s24>();
			}
			else if (-2 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_sd>();
			}
			else if (-1 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_sf>();
			}

			else return 0;
		}
		else if ( 1 == h.numchannels)
		{
			if ( 16 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_m16>();
			}
			else if ( 8 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_m8>();
			}
			else if (24 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_m24>();
			}
			else if (-2 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_md>();
			}
			else if (-1 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_mf>();
			}
			else return 0;
		}
		else if ( 4 == h.numchannels)
		{
			if ( 16 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_q16>();
			}
			else if ( 8 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_q8>();
			}
			else if (24 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_q24>();
			}
			else if (-2 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_qd>();
			}
			else if (-1 == h.samplesize)
			{
				return new typename instantiate_metafunc::template apply< sampletype_qf>();
			}
			else return 0;
		}

		else return 0;

	}
};
