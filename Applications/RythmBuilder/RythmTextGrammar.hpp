#ifndef __RYTHMTEXTGRAMMAR_HPP__
#define __RYTHMTEXTGRAMMAR_HPP__
#include <boost/spirit/qi.hpp>
#include <string>

template <typename Iterator>
struct rythm_grammar_def: boost::spirit::qi::grammar_def<Iterator, boost::spirit::ascii::space_type>
{
    typedef boost::spirit::qi::rule<Iterator, boost::spirit::ascii::space_type> regular_rule;

    rythm_grammar_def()
    {
        using namespace boost::spirit;
        using namespace boost::spirit::qi;
        using namespace boost::spirit::ascii;

        file    =   +track;
        track   =   header >> +bar;
        header  =   "TRACK" >> string_ >> !( '-' >> string_) >> ',' >> uint_ >> ',' >> (any_char - '\n')
        bar     =   bar_header >> ':' >> notes;
        notes   =   +(
                        note | nlet | measure_sep | pause | multiply | continuation
                    )
            ;
        continuation = lexeme[ '\n' >> space_type];
        nlet    =   uint_ >> !( '/' >> uint_) >> '(' >> notes >> ')';
        note    =   no_case[ note_symbols];
        note_symbols += "DO", "TA", "TU";
        measure_sep = char_('|');
        pause   =   char_('.');
        multiply=   '*' >> uint_;
    }

    regular_rule file, track, bar, notes, nlet, measure_sep, pause, multiply, note, header;
    boost::spirit::qi::symbols<char, std::string> note_symbols;

};

#endif //__RYTHMTEXTGRAMMAR_HPP__