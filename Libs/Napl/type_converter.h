//
// a type converter is a 'uniform_block_converter' that converts samples from one type to another
//
template< typename source_type, typename destination_type>
struct type_converter: public typed_converter< type_converter< source_type, destination_type>, source_type, destination_type>
{
	void do_convert( const source_type *src, destination_type *dest)
	{
		*dest = *src;
	}
};

template <typename source_type, typename destination_type>
struct type_conversion_mutator : public uniform_block_converter< type_converter< source_type, destination_type> >
{

};