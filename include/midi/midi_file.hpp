#if !defined(MIDI_FILE_HPP)
#define MIDI_FILE_HPP

#include <vector>
#include "midi_event_types.hpp"

typedef std::vector< events::timed_midi_event>   midi_track;

struct midi_header
{
    unsigned format;
    unsigned number_of_tracks;
    unsigned division;
};

struct midi_file
{
	typedef std::vector<midi_track> tracks_type;
    midi_header header;
    tracks_type tracks;
};

#endif //MIDI_FILE_HPP