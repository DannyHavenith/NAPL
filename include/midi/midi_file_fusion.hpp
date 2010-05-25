#ifndef MIDI_FILE_FUSION_HPP
#define MIDI_FILE_FUSION_HPP
#include "midi_events_fusion.hpp"
#include "midi_file.hpp"
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(
    midi_header,
    (unsigned, format)
    (unsigned, number_of_tracks)
    (unsigned, division)
 )

BOOST_FUSION_ADAPT_STRUCT(
    midi_file,
    (midi_header, header)
    (std::vector<midi_track>, tracks)
)


#endif //MIDI_FILE_FUSION_HPP