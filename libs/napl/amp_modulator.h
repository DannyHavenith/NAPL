// amplitude modulator
template< typename modulator_type, typename sampletype>
struct amp_modulator_mutator
{
	typedef sampletype sample_type;
	typedef modulator_type left_type;

	void MutateHeader( stream_header &, const stream_header &, const stream_header &)
	{
		// nop
	}

	sample_type Mutate( const modulator_type &mod, const sample_type &sample)
	{
		return sample_type( sample * mod);
	}

};
