// midi_parser.cpp
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

/// Binary grammar that parses midi files.
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

        // a midi file consists of a header chunk followed by zero or more track chunks
        file
            %= header >> *track
            ;

        // a header chunk constist of the signature 'MThd' followed by the chunk size (6)
        // The three words of the header are format, number of tracks and a time division value.
        header
            %= lit("MThd") >> big_dword(6) >> big_word >> big_word >> big_word
            ;

        // a track consists of the signature 'MTrk', followed by the chunk size (assigned to _a), followed by
        // timed midi events. The subrange directive is used to limit the parsing to only those bytes
        // that fall within the chunk (-size).
        track
            %= lit("MTrk") >>  omit[big_dword [_a = _1]] >> subrange(_a)[+timed_event]
            ;

        // a 'timed event' consist of a time stamp (the time between this event and the previous one) and a midi event
        timed_event
            %= variable_length_quantity >> event
            ;

        // midi events are either sysex events, meta-events or 'channel events'.
        event
            %= 
            (
                sysex_event
            |   meta_event
            |   channel_event
            )
            ;

        // a meta-event starts with 0xFF, followed by an event type, followed by a size inidicator followed by the indicated amount of bytes.
        meta_event
            %= omit[char_('\xff')] >> byte_ >> omit[variable_length_quantity[ _a = _1]] >> subrange(_a)[*byte_]
            ;

        // a sysex event starts with 0xF7 or 0xFE, followed by a size, followed by the indicated amount of bytes.
        sysex_event
			= (char_('\xfe') | char_('\xf7')) >> variable_length_quantity[ _a = _1] >> subrange(_a)[*byte_]
			;

        // a variable length quantity consist of zero or more bytes with the high bit set, followed by a single byte with a zero most significant bit.
        variable_length_quantity
            = eps[_val = 0] >> *high_byte[_val = (_val << 7) + (_1 & 0x7f)] >> low_byte[_val = (_val << 7) + _1]
            ;
        

        // channel events use a running status scheme, where the first byte of the event can be skipped if it is the same as the previous event.
        // if there's a high_byte, that will be the new event/channel and we store it in running_status.
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
        
        // Now follow the rules for the channel events (note-on, note-off, aftertouch, etc.).
        // Note that the first byte of the event has already been read by the channel_event rule and stored in
        // running_status. That is why all these rules first check the running status in an epsilon parser. These epsilon
        // parsers act as a condition and should be read as "if ( (running_status & 0xf0) == some value) { parse the event } else { don't parse }
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

        // note how the pitch bend value is little endian
        pitch_bend_event
            %=  eps( (ref(running_status) & 0xf0) == 0xe0) >> little_word;
            ;

        // convenience rules to distinguish between low bytes (0-127) and high bytes (128-255).
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
    rule<Iterator, events::timed_midi_event()   > timed_event;
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

/// parse the midi file that the istream 'in' refers to and return the result in a midi_file structure
/// Note that on some platforms the input file must have been opened as binary.
bool parse_midifile( std::istream &in, midi_file &result)
{

	result.tracks.clear();

	// don't skip whitespaces.
    in.unsetf( ios_base::skipws);

    // copy the whole file into a string.
    typedef std::istreambuf_iterator<char> base_iterator;
    typedef string::iterator iterator;
    std::string buffer;
    buffer.assign( base_iterator(in), base_iterator());

    // parse the string
    midi_parser<iterator> parser;
    iterator first = buffer.begin();
    iterator last = buffer.end();

	boost::spirit::qi::parse( first, last, parser, result);

	return first == last;
}
