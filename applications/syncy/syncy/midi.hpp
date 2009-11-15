#if !defined( SYNCY_MIDI_HPP)
#define SYNCY_MIDI_HPP
#include <boost/filesystem.hpp>
#include "lrc_parser.hpp"

lyrics::songtext parse_midi_text( const boost::filesystem::path &p, unsigned int channel);
#endif //SYNCY_MIDI_HPP