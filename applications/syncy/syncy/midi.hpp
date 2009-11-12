#if !defined( SYNCY_MIDI_HPP)
#define SYNCY_MIDI_HPP
#include <boost/filesystem.hpp>
namespace lyrics { struct text; }
// todo: implement
lyrics::songtext parse_midi_text( const boost::filesystem::path &p);
#endif //SYNCY_MIDI_HPP