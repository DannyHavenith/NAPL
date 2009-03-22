//
// a processor that creates a stereo signal from two mono input signals.
template <class mono_type>
struct stereo_maker : public binary_block_processor, private block_owner
{
	virtual void MutateHeader( stream_header &result, const stream_header &left, const stream_header &right)
	{
		result.numchannels *= 2;
	}

protected:
	typedef StereoSample< mono_type> stereo_type;
	typedef sample_container<mono_type> mono_container;
	virtual void ProcessBlocks( block_consumer *pC, const sample_block &left, const sample_block &right)
	{
		mono_container left_container( left);
		mono_container right_container( right);
		typename mono_container::iterator left_ptr = left_container.begin();
		typename mono_container::iterator right_ptr = right_container.begin();

		block_handle h( this); // releases the block on exit
		sample_block &block( h.get_block());

		block.m_start = block.buffer_begin();
		// nearest multiple of the output size...
		size_t max_block_size = (block.buffer_size()/ sizeof( stereo_type)) * sizeof( stereo_type);

		while (right_ptr < right_container.end())
		{
			stereo_type *output_ptr = ( stereo_type *)(block.buffer_begin());
			stereo_type *output_end = (stereo_type *)( block.buffer_begin() + max_block_size);
			size_t sample_count = 0;

			while (right_ptr != right_container.end() && output_ptr != output_end)
			{
				*output_ptr = stereo_type( *left_ptr, *right_ptr);
				++output_ptr;
				++left_ptr;
				++right_ptr;
				++sample_count;
			}
			block.m_end = block.m_start + (sample_count * sizeof( stereo_type));
			pC->ReceiveBlock( block);
		}
	}
};
