#ifndef __RYTHMTEXTGRAMMAR_HPP__
#define __RYTHMTEXTGRAMMAR_HPP__
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/bind/bind.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <vector>

#include "track_builder.h"

inline void AppendChar( std::string &str, char c)
{
    str.push_back(c);
}

template <typename Iterator>
struct rythm_grammar_def: boost::spirit::qi::grammar<Iterator, boost::spirit::standard::space_type>
{
    typedef boost::spirit::standard::space_type space_type;

    template< typename ConvertibleToStringRange>
    void set_note_symbols(  const ConvertibleToStringRange &range)
    {
        BOOST_FOREACH( std::string val, range)
        {
            boost::algorithm::to_lower( val);
            note_symbols.add( val.c_str(), val);
        }
    }

    template< typename ConvertibleToStringRange>
    void set_pause_symbols(  const ConvertibleToStringRange &range)
    {
        BOOST_FOREACH( std::string val, range)
        {
            boost::algorithm::to_lower( val);
            pause_symbols.add( val.c_str());
        }
    }

    template< typename ConvertibleToStringRange>
    rythm_grammar_def( const ConvertibleToStringRange &range, track_builder &builder)
     :boost::spirit::qi::grammar<Iterator, boost::spirit::standard::space_type>(file)
    {
        using namespace boost::placeholders;
        using namespace boost::spirit;
        using namespace boost::spirit::qi;
        using boost::phoenix::bind;
        using boost::spirit::qi::_1;
        using boost::spirit::qi::_2;

        set_note_symbols( range);



        BOOST_SPIRIT_DEBUG_NODE(file);
        file    =   +track                  [bind(&track_builder::end_track, &builder)];

        BOOST_SPIRIT_DEBUG_NODE(track);
        track   =   -header >> +bar         [bind(&track_builder::end_bar, &builder)];

        BOOST_SPIRIT_DEBUG_NODE( header);
        header  =       lit("TRACK")
                            >   title_string             [_a = _1]
                            >>  -( '-' >> title_string   [_b = _1])
                            >>  ','
                            >   uint_               [_c = _1]
                            >   ','
                            >>  lexeme[*(char_ - '\n')[_d += _1] ]
                                                    [boost::phoenix::bind(&track_builder::start_track,
                                                            &builder,
                                                            _a,
                                                            _b,
                                                            _c,
                                                            _d
                                                            )
                                                    ];

        BOOST_SPIRIT_DEBUG_NODE( bar);
        bar     =    opt_bar_header >>  notes;

        BOOST_SPIRIT_DEBUG_NODE( opt_bar_header);
        opt_bar_header = -char_(':') >> (string_ [_a = _1] >> -('(' >> string_[_b = _1] >> ')') >> ':')
                                                    [boost::phoenix::bind( &track_builder::start_bar,
                                                            &builder,
                                                            _a,
                                                            _b
                                                         )
                                                    ]
        |   (eps >> ':') [bind( &track_builder::start_bar, &builder, "djembe1", "")]
                        ;

        BOOST_SPIRIT_DEBUG_NODE( string_);
        string_  =   lexeme[ +alnum[boost::phoenix::bind( &AppendChar, _val, _1)]];

        title_string = lexeme[ +(char_ - ',' - '\n' - '-')[bind( &AppendChar, _val, _1)]];

        notes   =   +(
                        note | nlet | measure_sep | pause | multiply | anacrusis_marker
                    )
            ;

        anacrusis_marker =  lit('>') [bind(&track_builder::mark_anacrusis, &builder)];

        nlet    =       uint_                           [_a = _1]
                    >>  (-( '/' >>  uint_ [_b = _1])) [bind(    &track_builder::start_nlet,
                                                                &builder,
                                                                _a,
                                                                _b
                                                            )
                                                        ]
                    >>  ('(' >> notes >> ')')       [bind(&track_builder::end_nlet, &builder)]
                 ;

        note    =   no_case[ note_symbols]          [boost::phoenix::bind(&track_builder::append_note, &builder, _1)];
        measure_sep =
                        char_('|')                  [bind(&track_builder::new_measure, &builder)]
            ;

        pause   =   no_case[ pause_symbols]         [bind(&track_builder::pause, &builder)];
        pause_symbols.add( ".")("en");

        multiply=   '*' >> uint_                    [boost::phoenix::bind(&track_builder::repeat, &builder, _1)];

    }

    typedef boost::spirit::qi::rule<Iterator, space_type> regular_rule;
    typedef boost::spirit::qi::rule<Iterator, std::string(), space_type> string_rule;


    regular_rule file, track, bar, notes, measure_sep, pause, note, anacrusis_marker;
    regular_rule  multiply, continuation;

    boost::spirit::qi::rule<
        Iterator,
        boost::spirit::locals< std::string, std::string>,
        space_type>
            opt_bar_header;

    boost::spirit::qi::rule<
        Iterator,
        boost::spirit::locals< std::string, std::string, int, std::string>,
        space_type>
            header;


    boost::spirit::qi::rule<
        Iterator,
        boost::spirit::locals< unsigned int, int>,
        space_type>
            nlet;


    string_rule string_;
    string_rule title_string;

    boost::spirit::qi::symbols<char,std::string> note_symbols;
    boost::spirit::qi::symbols<char,int> pause_symbols;

};

#endif //__RYTHMTEXTGRAMMAR_HPP__
