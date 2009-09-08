// midi_parser.cpp : Defines the entry point for the console application.
//
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/spirit/include/qi_binary.hpp>

#include <boost/lambda/lambda.hpp>
#include "subsection_parser.hpp"
#include "midi_event_types.hpp"

//using namespace boost::phoenix;
using namespace boost::spirit;
using namespace boost::spirit::qi;
using namespace boost::spirit::ascii; // for a definition of space_type
using namespace boost::spirit::arg_names;
using namespace std;

namespace events
{
template<typename Derived>
struct visitor : public boost::static_visitor<>
{
    Derived &derived()
    {
        return *static_cast<Derived*>(this);
    }

    const Derived &derived() const
    {
        return *static_cast<Derived*>(this);
    }

    void operator()( const any &event)
    {
        boost::apply_visitor( derived(), event);
    }

    void operator()( const channel_event &event)
    {
        current_channel = event.channel;
        boost::apply_visitor( derived(), event.event);
    }

    void operator()( const meta &)
    {
    }

    void operator()( const sysex &)
    {
    }

    void operator()( const note_on &){}
    void operator()( const note_off &){}
    void operator()( const note_aftertouch &){}
    void operator()( const controller &){}
    void operator()( const program_change &){}
    void operator()( const channel_aftertouch &){}
    void operator()( const pitch_bend &){}

    unsigned short current_channel;
};

} // namespace events

template<typename Derived>
struct timed_midi_visitor : public events::visitor<Derived>
{
    timed_midi_visitor()
        :current_time(0)
    {

    }

    using events::visitor<Derived>::operator();
    void operator()( const events::timed_midi_event &event)
    {
        current_time += event.delta_time;
        derived()( event.event);
    }

    void reset()
    {
        current_time = 0;
    }

    size_t current_time;
};

struct print_text_visitor : public timed_midi_visitor<print_text_visitor>
{
    using timed_midi_visitor<print_text_visitor>::operator();

    void operator()( const events::note_on &note)
    {
        std::cout << "n:" << static_cast<int>( note.number);
    }

    void operator()( const events::note_off &note)
    {
        std::cout << "o:" << static_cast<int>( note.number);
    }
    
    void operator()( const events::meta &event)
    {
        if (event.type == 0x05 || (event.type == 0x01))
        {
            std::cout << current_time << '\t' << std::string( event.bytes.begin(), event.bytes.end()) << '\n';
        }
    }
};

typedef std::vector< events::timed_midi_event>   midi_track;

struct midi_header
{
    unsigned format;
    unsigned number_of_tracks;
    unsigned division;
};

BOOST_FUSION_ADAPT_STRUCT(
    midi_header,
    (unsigned, format)
    (unsigned, number_of_tracks)
    (unsigned, division)
 )

struct midi_file
{
    midi_header header;
    std::vector< midi_track> tracks;
};

BOOST_FUSION_ADAPT_STRUCT(
    midi_file,
    (midi_header, header)
    (std::vector<midi_track>, tracks)
)

template<typename Iterator>
struct midi_parser: grammar< Iterator, midi_file()>
{
    midi_parser() : 
        midi_parser::base_type( file),
            running_status(-1)
    {
        using boost::phoenix::val;
//        using boost::phoenix::var;
        using boost::phoenix::ref;
        using boost::phoenix::at_c;
        using boost::lambda::var;
        using boost::lambda::constant;

        file
            %= header >> *track
            ;

        header
            %= lit("MThd") >> big_dword(6) >> big_word >> big_word >> big_word
            ;

        track
            %= lit("MTrk") >>  omit[big_dword [_a = _1]] >> subsection(_a)[+event_group]
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
            %= char_('\xff') >> byte >> omit[variable_length_quantity[ _a = _1]] >> subsection(_a)[*byte]
            ;

        sysex_event
			= (char_('\xfe') | char_('\xf7')) >> variable_length_quantity[ _a = _1] >> subsection(_a)[*byte]
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
            %=  eps( (ref(running_status) & 0xf0) == 0x90) >> byte >> byte;

 
        note_off_event
            %=  eps( (ref(running_status) & 0xf0) == 0x80) >> byte >> byte
            ;
        
        note_aftertouch_event
            %=  eps( (ref(running_status) & 0xf0) == 0xa0) >> byte >> byte
            ;
        
        controller_event
            %=  eps( (ref(running_status) & 0xf0) == 0xb0) >> byte >> byte
            ;
        
        program_change_event
            %=  eps( (ref(running_status) & 0xf0) == 0xc0) >>  byte 
            ;
        
        channel_aftertouch_event
            %=  eps( (ref(running_status) & 0xf0) == 0xd0) >>  byte
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
            /**/
    }

    int running_status;
    rule<Iterator, unsigned char()                > high_byte;
    rule<Iterator, unsigned char()                > low_byte;
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

template<typename Expr>
bool test_validity( const Expr &)
{
	return boost::spirit::qi::is_valid_expr<Expr>::value;
}

template <typename Iterator>
struct sub : grammar<Iterator, std::vector<int>(), locals<size_t> >
{
    sub() : sub::base_type(start)
    {
        start %= omit[ byte[_a = _1]] >> subsection(_a)[ *byte];
    	//start %= *byte;
    }
    rule<Iterator, std::vector<int>(), locals<size_t> > start;
};

int main(int argc, char* argv[])
{
    using namespace std;

    if (argc < 2)
    {
        cerr << "usage: parsemidi <filename>\n";
        return -1;
    }

    ifstream in(argv[1], ios::binary);
    in.unsetf( ios_base::skipws);

    if (!in)
    {
        cerr << "could not open " << argv[1] << '\n';
        return -2;
    }
//    typedef boost::spirit::multi_pass<istream_iterator<char> > default_multi_pass_t;
    typedef std::istreambuf_iterator<char> base_iterator_type;
//    typedef multi_pass<base_iterator_type, unused_type> iterator;

    std::string buffer;
    buffer.assign( base_iterator_type(in), base_iterator_type());
    std::cout << buffer.size() << "\n";
    
    typedef string::iterator iterator;

    midi_parser<iterator> parser;
    midi_file contents;

    iterator first = buffer.begin();
    iterator last = buffer.end();
    boost::spirit::qi::parse( first, last, parser, contents);

    std::cout << "complete: " << (first == last) << std::endl;

    typedef std::vector< midi_track>::iterator track_iterator;
    for (track_iterator i = contents.tracks.begin(); i != contents.tracks.end(); ++i)
    {
        print_text_visitor v;
        std::for_each( i->begin(), i->end(), v);
    }

	return 0;
}

