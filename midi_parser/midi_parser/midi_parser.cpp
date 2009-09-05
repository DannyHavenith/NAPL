// midi_parser.cpp : Defines the entry point for the console application.
//
#include <string>
#include <vector>

#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/spirit/include/qi_binary.hpp>

#include "subsection_parser.hpp"

//using namespace boost::phoenix;
using namespace boost::spirit;
using namespace boost::spirit::qi;
using namespace boost::spirit::ascii; // for a definition of space_type
using namespace boost::spirit::arg_names;

struct midi_meta_event
{
};

struct midi_sysex_event
{
};

struct simple_midi_event
{
};

typedef boost::variant<
    midi_meta_event,
    midi_sysex_event,
    simple_midi_event
> midi_event_variant;

struct midi_event
{
    unsigned delta_time;
    midi_event_variant event;
};

BOOST_FUSION_ADAPT_STRUCT(
    midi_event,
    (unsigned, delta_time)
    (midi_event_variant, event)
    )

typedef std::vector< midi_event>   midi_track;

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
    midi_parser() : midi_parser::base_type( file)
    {
        file
            %= header >> +track
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
                midi_event_r
            |   sysex_event
            |   meta_event
            ;

        meta_event
            = byte(0xff) >> variable_length_quantity[ _a = _1] >> subsection(_a)[*byte]
            ;

        sysex_event
			= (byte(0xfe) | byte(0xf7)) >> variable_length_quantity[ _a = _1] >> subsection(_a)[*byte]
			;

        variable_length_quantity
            = eps[_val = 0] >> *high_byte[_val += _1] >> low_byte[_val += _1]
            ;

        using boost::phoenix::val;
        low_byte
            %= char_( val(0), val(127));

        high_byte
            %= char_( val(128), val(255));


    }

    rule<Iterator, int()>                 high_byte;
    rule<Iterator, int()>                 low_byte;
    rule<Iterator, midi_file()          > file;
    rule<Iterator, midi_header()        > header;
    rule<Iterator, midi_track(size_t)   > track_data;
    rule<Iterator, midi_event()         > event_group;
    rule<Iterator, midi_event_variant() > event;
    rule<Iterator, simple_midi_event()  > midi_event_r;
    rule<Iterator, midi_track(),        locals<size_t> > track;
    rule<Iterator, midi_sysex_event(),  locals<size_t> > sysex_event;
    rule<Iterator, midi_meta_event(),   locals<size_t> > meta_event;
    rule<Iterator, size_t()            > variable_length_quantity;
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
    string duck("\002\002\003\004");
    typedef string::iterator iterator;

//    sub<iterator> sub_parser;
    midi_parser<iterator> sub_parser;

//    std::vector<int> result;

    boost::spirit::qi::parse( duck.begin(), duck.end(), sub_parser);

  //  std::cout << result.size() << ' ' << result[0] << ' ' << result[1] << std::endl;

	return 0;
}

