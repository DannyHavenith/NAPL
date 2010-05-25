#ifndef MIDI_EVENTS_FUSION_HPP
#define MIDI_EVENTS_FUSION_HPP
#include <boost/fusion/include/adapt_struct.hpp>
#include "midi_event_types.hpp"
BOOST_FUSION_ADAPT_STRUCT(
    events::note_on,
    (unsigned char, number)
    (unsigned char, velocity)
    )

BOOST_FUSION_ADAPT_STRUCT(
    events::meta,
    (unsigned char, type)
    (std::vector<unsigned char>, bytes)
    )

BOOST_FUSION_ADAPT_STRUCT(
    events::note_off,
    (unsigned char, number)
    (unsigned char, velocity)
    )
BOOST_FUSION_ADAPT_STRUCT(
    events::note_aftertouch,
    (unsigned char, number)
    (unsigned char, velocity)
    )

BOOST_FUSION_ADAPT_STRUCT(
    events::controller,
    (unsigned char, which)
    (unsigned char, value)
    )

BOOST_FUSION_ADAPT_STRUCT(
    events::program_change,
    (unsigned char, program)
    )

BOOST_FUSION_ADAPT_STRUCT(
    events::channel_aftertouch,
    (unsigned char, value)
    )

BOOST_FUSION_ADAPT_STRUCT(
    events::pitch_bend,
    (unsigned short, value)
    )

BOOST_FUSION_ADAPT_STRUCT(
    events::channel_event,
    (unsigned char, channel)
    (events::channel_event_variant, event)
    )

BOOST_FUSION_ADAPT_STRUCT(
    events::timed_midi_event,
    (unsigned, delta_time)
    (events::any, event)
    )


#endif //MIDI_EVENTS_FUSION_HPP