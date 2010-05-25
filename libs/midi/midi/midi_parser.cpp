// midi_parser.cpp : Defines the entry point for the console application.
//
#include <string>
#include <iostream>
#include <iterator>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi_binary.hpp>

#include "subrange_parser.hpp"

#include "midi/midi_events_fusion.hpp"
#include "midi/midi_file_fusion.hpp"
#include "midi/midi_parser.hpp"

using namespace boost::phoenix;
using namespace boost::spirit;
using namespace std;
using namespace boost::spirit::qi;

template<typename Iterator>
struct midi_parser: grammar< Iterator, midi_file()>
{
    midi_parser() : 
        midi_parser::base_type( file),
            running_status(-1)
    {
        using boost::spirit::ascii::char_;
        using boost::phoenix::ref;
        using boost::phoenix::at_c;

        file
            %= header >> *track
            ;

        header
            %= lit("MThd") >> big_dword(6) >> big_word >> big_word >> big_word
            ;

        track
            %= lit("MTrk") >>  omit[big_dword [_a = _1]] >> subrange(_a)[+event_group]
            ;

        event_group
            %= variable_length_quantity >> event
            ;

        event
            %= 
            (
                sysex_event
            |   meta_event
            |   channel_event
            )
            ;

        meta_event
            %= char_('\xff') >> byte_ >> omit[variable_length_quantity[ _a = _1]] >> subrange(_a)[*byte_]
            ;

        sysex_event
			= (char_('\xfe') | char_('\xf7')) >> variable_length_quantity[ _a = _1] >> subrange(_a)[*byte_]
			;

        variable_length_quantity
            = eps[_val = 0] >> *high_byte[_val = (_val << 7) + (_1 & 0x7f)] >> low_byte[_val = (_val << 7) + _1]
            ;
        

        // if there's a high_byte, that will be the event/channel
        // if there isn't, we use the running status (the previously seen event).
        channel_event
            =  omit[-high_byte[ref(running_status) = _1]] >> 
                    (
                        note_off_event
                    |   note_on_event
                    |   note_aftertouch_event
                    |   controller_event
                    |   program_change_event
                    |   channel_aftertouch_event
                    |   pitch_bend_event
                    ) 
                    [
                        at_c<1>(_val) = _1
                    ]
                    [
                        at_c<0>(_val) = ref(running_status) & 0x0f
                    ]
            ;
        
        note_on_event
            %=  eps( (ref(running_status) & 0xf0) == 0x90) >> byte_ >> byte_
            ;

        note_off_event
            %=  eps( (ref(running_status) & 0xf0) == 0x80) >> byte_ >> byte_
            ;
        
        note_aftertouch_event
            %=  eps( (ref(running_status) & 0xf0) == 0xa0) >> byte_ >> byte_
            ;
        
        controller_event
            %=  eps( (ref(running_status) & 0xf0) == 0xb0) >> byte_ >> byte_
            ;
        
        program_change_event
            %=  eps( (ref(running_status) & 0xf0) == 0xc0) >>  byte_ 
            ;
        
        channel_aftertouch_event
            %=  eps( (ref(running_status) & 0xf0) == 0xd0) >>  byte_
            ;

        pitch_bend_event
            %=  eps( (ref(running_status) & 0xf0) == 0xe0) >> little_word;
            ;

        low_byte
            %= char_( '\x00', '\x7f')
            ;

        high_byte
            %= char_( '\x80', '\xff')
            ;

    }

    int running_status;
    rule<Iterator, unsigned char()      > high_byte;
    rule<Iterator, unsigned char()      > low_byte;
    rule<Iterator, midi_file()          > file;
    rule<Iterator, midi_header()        > header;
    rule<Iterator, midi_track(size_t)   > track_data;
    rule<Iterator, events::timed_midi_event()   > event_group;
    rule<Iterator, events::any()        > event;
    rule<Iterator, events::pitch_bend() > pitch_bend_event;
    rule<Iterator, size_t()             > variable_length_quantity;
    rule<Iterator, events::note_off()       > note_off_event;
    rule<Iterator, events::note_on()        > note_on_event;
    rule<Iterator, events::note_aftertouch()> note_aftertouch_event;
    rule<Iterator, events::controller()     > controller_event;
    rule<Iterator, events::program_change() > program_change_event;
    rule<Iterator, events::channel_aftertouch()> channel_aftertouch_event;
    rule<Iterator, midi_track(),            locals<size_t>  > track;
    rule<Iterator, events::sysex(),         locals<size_t>  > sysex_event;
    rule<Iterator, events::meta(),          locals<size_t>  > meta_event;
    rule<Iterator, events::channel_event(), locals<unsigned int>     > channel_event;
};

bool parse_midifile( std::istream &in, midi_file &result)
{

	result.tracks.clear();
    in.unsetf( ios_base::skipws);

    typedef std::istreambuf_iterator<char> base_iterator;
    typedef string::iterator iterator;

    std::string buffer;
    midi_parser<iterator> parser;
    buffer.assign( base_iterator(in), base_iterator());
    iterator first = buffer.begin();
    iterator last = buffer.end();

	boost::spirit::qi::parse( first, last, parser, result);

	return first == last;
}
