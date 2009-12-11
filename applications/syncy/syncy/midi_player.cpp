#include "stdafx.h"
#include <stdexcept>
#include <boost/bind.hpp>
#include <algorithm> // for for_each
#include <fstream>

#include "midi_player.hpp"
#include "midi/timed_midi_visitor.hpp"
#include "midi/event_to_codes.hpp"
#include "midi/midi_parser.hpp"
#include "midi/midi_multiplexer.hpp"

struct midi_exception : public std::runtime_error
{
	midi_exception( const std::string &what)
		:std::runtime_error( what)
	{}

};

struct midi_player::implementation
{
    HMIDIOUT midi_handle;
};

midi_player::midi_player( )
    :impl( new implementation)
{
    reset();
    if (midiOutOpen( &impl->midi_handle, (UINT)-1, 0,0, CALLBACK_WINDOW))
    {
        impl.reset(); // have to reset, since d'tor will not be called
        throw midi_exception( "could not open midi device");
    }
}

/// this constructor is only defined so that the implementation
/// of the scoped_ptr destructor is in this file...
midi_player::~midi_player()
{
}

void midi_player::display( lyrics::centisecond position)
{
    while (current != events.end() && current->first <= position)
    {
        emit_codes( current->second);
        ++current;
    }
}

void midi_player::emit_codes( const codes &midi_codes)
{
    std::for_each( 
        midi_codes.begin(), midi_codes.end(),
        boost::bind<MMRESULT>( midiOutShortMsg, impl->midi_handle, _1)
        );
}

struct midi_events_builder : public events::timed_visitor< midi_events_builder>
{
    typedef events::timed_visitor< midi_events_builder> parent;

    midi_events_builder( event_map &result_, const midi_header &h, double delay_)
        :parent(h), result( result_), delay( delay_)
    {
    }

    using parent::operator ();
    void operator()( const events::channel_event &event)
    {
        events::event_to_code_translator trans;
        trans( event);

        if (trans.length)
        {
            boost::uint32_t word = 0;
            for (int i = trans.length - 1; i >= 0; --i)
            {
                word = (word << 8) | trans.codes[i];
            }

            double time = current_time + delay;
            if (time < 0.0)
            {
                time = 0.0;
            }

            result[ static_cast<unsigned int>(time * 100)]
                .push_back( word);
        }
    }
private:
    double    delay;
    event_map &result;
};

void midi_player_from_file( const boost::filesystem::path &p, midi_player &result, double delay)
{
	using namespace std;

	ifstream in(p.string().c_str(), ios::binary);

	if (!in)
	{
		throw std::runtime_error( "could not open file " + p.string());
	}

	midi_file contents;
	if (!parse_midifile( in, contents))
	{
        throw std::runtime_error( "corrupt midi file " + p.string());
	}
	else
	{

        // merge all tracks into one track
		midi_multiplexer multiplexer( contents.tracks);
        event_map events;
        
        // and then offer all events to the text extractor.
		multiplexer.accept( midi_events_builder( events, contents.header, delay));
        result.set_events( events);
        
	}

}