//
// to be used in a uniform_block_converter
template< typename sampletype>
struct extract_channel: public typed_converter< extract_channel<sampletype>, sampletype, typename sampletraits<sampletype>::channel_type >
{
private:
	short m_channel;

public:
	extract_channel( short channel)
		:m_channel( channel)
	{
		// nop
	}

	typedef typename sampletraits<sampletype>::channel_type mono_type;
	void do_convert( const sampletype *source_ptr, mono_type *dest_ptr)
	{
		sampletraits<sampletype>::extract_channel( *source_ptr, *dest_ptr, m_channel);
	}
};

template< typename stereo_type>
struct channel_extract_mutator : public uniform_block_converter< extract_channel< stereo_type> >
{

	channel_extract_mutator( short channel)
		: uniform_block_converter< extract_channel< stereo_type> >( channel)
	{
	}

};
