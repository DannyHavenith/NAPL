template<typename sampletype>
struct mut_negator
{
public:

	typedef sampletype sample_type;
	typedef int result_information_type;

	inline void MutateHeader( stream_header &)
	{
		// nop
	}

	// negate the sample value.
	sample_type operator()( const sample_type &p) const
	{
		return sampletraits<sample_type>::negate( p);
	}

};

template< typename sampletype>
class negator : public uniform_block_mutator< mut_negator< sampletype> >
{
	// nop
};
