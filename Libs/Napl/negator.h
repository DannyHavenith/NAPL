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
	inline void Mutate( sample_type *p)
	{
		*p = sampletraits<sample_type>::negate( *p);
	}

	inline int GetResult() { return 0;}
};

template< typename sampletype>
class negator : public uniform_block_mutator< mut_negator< sampletype> >
{
	// nop
};