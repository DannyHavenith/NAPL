#ifndef MIDI_EVENT_TYPES_HPP
#define MIDI_EVENT_TYPES_HPP

#include <boost/variant.hpp>

namespace events
{
    struct note
    {
        unsigned char number;
        unsigned char velocity;
    };


    struct note_off : note
    {};

    struct note_on : note
    {};

    struct note_aftertouch : note
    {};

    struct controller 
    {
        unsigned char which;
        unsigned char value;
    };

    struct program_change
    {
        program_change &operator=( int rhs)
        {
            program = rhs;
        }
        unsigned char program;
    };

    struct channel_aftertouch
    {
        unsigned char value;
    };
    struct pitch_bend
    {
        unsigned short value;
    };

    typedef boost::variant<
        note_on,
        note_off,
        note_aftertouch,
        controller,
        program_change,
        channel_aftertouch,
        pitch_bend> channel_event_variant;

    struct channel_event
    {
        unsigned char         channel;
        channel_event_variant event;
    };

    struct meta
    {
        unsigned char       type;
        std::vector<unsigned char>   bytes;
    };

    struct sysex
    {
    };

    typedef boost::variant<
        meta,
        sysex,
        channel_event
    > any;

    struct timed_midi_event
    {
        unsigned delta_time;
        events::any event;
    };
} // namespace events

#endif // MIDI_EVENT_TYPES_HPP