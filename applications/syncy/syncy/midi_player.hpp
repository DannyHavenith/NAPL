#if !defined( MIDI_PLAYER_HPP)
#define MIDI_PLAYER_HPP
#include <vector>
#include <map>
#include <boost/cstdint.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem/path.hpp>

#include "text_player.hpp"

typedef size_t centisecond;
typedef std::vector< boost::uint32_t> codes;
typedef std::map< centisecond, codes> event_map;

class midi_player : public text_player_interface
{
public:

    
    midi_player( );

    void set_events( event_map &events_)
    {
        events = events_;
        reset();
    }

    void reset()
    {
        current = events.begin();
    }

    ~midi_player();

    virtual void display( lyrics::centisecond position);

private:

    void emit_codes( const codes &midi_codes);

private:
    struct implementation; ///< pimpl, contains OS dependent stuff
    boost::scoped_ptr<implementation> impl;

    event_map events;
    event_map::const_iterator current;

};

void midi_player_from_file( const boost::filesystem::path &p, midi_player &result);

#endif //MIDI_PLAYER_HPP