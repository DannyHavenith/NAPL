//
// a type converter is a 'uniform_block_converter' that converts samples from one type to another
//
template< typename source_type, typename destination_type>
struct type_converter: public typed_converter< type_converter< source_type, destination_type>, source_type, destination_type>
{

	typedef sampletraits<source_type> source_traits;
	typedef sampletraits<destination_type> dest_traits;
	typedef sampletraits< typename source_traits::channel_type> source_channel_traits;
	typedef sampletraits< typename dest_traits::channel_type> dest_channel_traits;

	typedef typename source_traits::template foreach_channel_type<double>::type double_type;

	void do_convert( const source_type *src, destination_type *dest)
	{
		static const double factor =
			(double(dest_channel_traits::get_middle()) - double( dest_channel_traits::get_min()))/
			(double(source_channel_traits::get_middle()) - double( source_channel_traits::get_min()));

		round(
			(double_type( *src) - double_type(source_traits::get_middle())) * factor
			+ double_type(dest_traits::get_middle()), *dest);
	}

private:
	static void round( double_type source, destination_type &dest)
	{
		dest = source;// + dest_traits::expand_to_channels( (source > 0.0)?0.5:-0.5);
	}

};

template <typename source_type, typename destination_type>
struct type_conversion_mutator : public uniform_block_converter< type_converter< source_type, destination_type> >
{

};
