#ifndef __RYTHMTEXTGRAMMAR_HPP__
#define __RYTHMTEXTGRAMMAR_HPP__
#include <boost/spirit/qi.hpp>
#include <boost/spirit/phoenix/core.hpp>
#include <boost/spirit/phoenix/operator.hpp>
#include <boost/spirit/phoenix/bind/bind_member_function.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <vector>

#include "track_builder.h"

template <typename Iterator>
struct rythm_grammar_def: boost::spirit::qi::grammar_def<Iterator, boost::spirit::ascii::space_type>
{
    typedef boost::spirit::ascii::space_type space_type;

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
    {
        using namespace boost::spirit;
        using namespace boost::spirit::qi;
        using namespace boost::spirit::ascii;
        using namespace boost::spirit::arg_names;
        using namespace boost::phoenix;

        set_note_symbols( range);


        
        BOOST_SPIRIT_DEBUG_NODE(file);
        file    =   +track                  [bind(&track_builder::end_track, ref(builder))];
        
        BOOST_SPIRIT_DEBUG_NODE(track);
        track   =   -header >> +bar         [bind(&track_builder::end_bar, ref(builder))];
        
        BOOST_SPIRIT_DEBUG_NODE( header);
        header  =       lit("TRACK") 
                            >   string_             [_a = _1] 
                            >>  -( '-' >> string_   [_b = _1]) 
                            >>  ',' 
                            >   uint_               [_c = _1]
                            >   ',' 
                            >>  lexeme[*(char_ - '\n')[_d += _1] ] 
                                                    [bind(&track_builder::start_track,
                                                            ref(builder),
                                                            _a,
                                                            _b,
                                                            _c,
                                                            _d
                                                            )
                                                    ];
        
        BOOST_SPIRIT_DEBUG_NODE( bar);
        bar     =    opt_bar_header >>  notes;

        BOOST_SPIRIT_DEBUG_NODE( opt_bar_header);
        opt_bar_header = -char_(':') >> (-(string_ [_a = _1] >> -('(' >> string_[_b = _1] >> ')') >> ':'))
                                                    [bind( &track_builder::start_bar,
                                                            ref( builder),
                                                            _a,
                                                            _b
                                                         )
                                                    ]
                        ;

        BOOST_SPIRIT_DEBUG_NODE( string_);
        string_  =   lexeme[ +alnum[_val += _1]];

        notes   =   +(
                        note | nlet | measure_sep | pause | multiply 
                    )
            ;
            
        nlet    =       uint_                           [_a = _1] 
                    >>  (-( '/' >>  uint_ [_b = _1])) [bind(    &track_builder::start_nlet,
                                                                ref( builder),
                                                                _a,
                                                                _b
                                                            )
                                                        ]
                    >>  ('(' >> notes >> ')')       [bind(&track_builder::end_nlet, ref(builder))]
                 ;
        
        note    =   no_case[ note_symbols]          [bind(&track_builder::note, ref(builder), _1)];
        measure_sep = 
                        char_('|')                  [bind(&track_builder::new_measure, ref(builder))]
            ;                    

        pause   =   no_case[ pause_symbols]         [bind(&track_builder::pause, ref(builder))];
        pause_symbols.add( ".")("en");

        multiply=   '*' >> uint_                    [bind(&track_builder::repeat, ref(builder), _1)];
        
    }

    typedef boost::spirit::qi::rule<Iterator, space_type> regular_rule;
    typedef boost::spirit::qi::rule<Iterator, std::string(), space_type> string_rule;


    regular_rule file, track, bar, notes, measure_sep, pause, note;
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
        boost::spirit::locals< int, int>, 
        space_type>
            nlet;


    string_rule string_;
    boost::spirit::qi::symbols<char,std::string> note_symbols;
    boost::spirit::qi::symbols<char,int> pause_symbols;

};

#endif //__RYTHMTEXTGRAMMAR_HPP__