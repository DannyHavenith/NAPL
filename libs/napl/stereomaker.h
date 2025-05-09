#ifndef STEREOMAKER_H
#define STEREOMAKER_H
#include "samplebl.h"
#include "samplety.h"

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

    sample_block ProcessBlocks( const sample_block &left, const sample_block &right) override
    {
        assert( left.size() == right.size());
        mono_container left_container( left);
        mono_container right_container( right);

        auto left_ptr = left_container.begin();
        auto right_ptr = right_container.begin();

        sample_block block_destination( get_block());

        // make sure the output buffer contains at most as many samples as either input buffer
        size_t max_sample_count = std::min(block_destination.size()/ sizeof( stereo_type), left_container.size());
        block_destination.Truncate(max_sample_count * sizeof( stereo_type));

        stereo_type *output_ptr = ( stereo_type *)(block_destination.buffer_begin());
        stereo_type *output_end = ((stereo_type *)( block_destination.buffer_begin())) + max_sample_count;

        while (output_ptr != output_end)
        {
            *output_ptr = stereo_type( *left_ptr, *right_ptr);
            ++output_ptr;
            ++left_ptr;
            ++right_ptr;
        }
        return block_destination;
    }
};

#endif // STEREOMAKER_H
