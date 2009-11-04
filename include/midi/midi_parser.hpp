#if !defined( MIDI_PARSER_HPP)
#define MIDI_PARSER_HPP
#include <iosfwd>
#include "midi_file.hpp"

bool parse_midifile( std::istream &stream, midi_file &result);

#endif //MIDI_PARSER_HPP