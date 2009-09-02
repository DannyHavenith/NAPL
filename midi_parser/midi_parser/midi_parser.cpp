// midi_parser.cpp : Defines the entry point for the console application.
//
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>

//using namespace boost::phoenix;
using namespace boost::spirit;
using namespace boost::spirit::qi;
//using namespace boost::spirit::ascii;
using namespace boost::spirit::arg_names;


struct midi_track
{
};

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
    (std::vector<midi_tracks>, tracks)
)

template<Iterator>
struct midi_parser: grammar< Iterator, midi_file(), space_type>
{
    midi_parser() : midi_parser::base_type( file)
    {
        file 
            %= header >> +track
            ;

        header 
            %= lit("MThd") > big_dword(6) >> big_word >> big_word >> big_word
            ;

        track
            %= lit("MTrk") >  big_dword [_a = _1] >> track_data( _a)
            ;



        event_group
            %= variable_length_number >> event
            ;

        event
            %=
                midi_event
            |   sysex_event
            |   meta_event
            ;

    }

    rule<Iterator, midi_file()>                     file;
    rule<Iterator, midi_track(), locals<size_t> >   track;
    rule<Iterator, midi_header()>                   header;
    rule<Iterator, std::vector<midi_event>(size_t)> track_data;

    rule<Iterator, unsigned>             variable_length_number;
};

template<typename InputIterator>
struct offset_iterator
{
    offset_iterator() 
        :offset(0)
    {
    }
    
    offset_iterator( InputIterator i, size_t o)
        :it(i), offset(o)
    {
    }

    offset_iterator( offset_iterator const &other)
        :it( other.it), offset( other.offset)
    {
    }

    size_t offset() const
    {
        return offset;
    }

    offset_iterator &operator++()
    {
        ++offset;
        ++it;
        return *this;
    }

private:
    InputIterator it;
    size_t offset;
};

int main(int argc, char* argv[])
{
	return 0;
}

