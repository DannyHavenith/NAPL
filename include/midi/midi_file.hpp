#if !defined(MIDI_FILE_HPP)
#define MIDI_FILE_HPP

#include <vector>
#include <boost/fusion/include/adapt_struct.hpp>
#include "midi_event_types.hpp"

typedef std::vector< events::timed_midi_event>   midi_track;

struct midi_header
{
    unsigned format;
    unsigned number_of_tracks;
    unsigned division;
};

BOOST_FUSION_ADAPT_STRUCT(
    midi_header,
    (unsigned, format)
    (unsigned, number_of_tracks)
    (unsigned, division)
 )

struct midi_file
{
    midi_header header;
    std::vector< midi_track> tracks;
};

BOOST_FUSION_ADAPT_STRUCT(
    midi_file,
    (midi_header, header)
    (std::vector<midi_track>, tracks)
)

#endif //MIDI_FILE_HPP