#include "stdafx.h"
#include <boost/assign.hpp>
#include <boost/spirit/qi.hpp>

#include "RythmParser.h"
#include "track_builder.h"
#include "RythmTextGrammar.hpp"

bool ParseRythm(  const boost::filesystem::path &instrument_path, const std::string &content)
{
    using namespace boost::spirit;
    using namespace boost::spirit::qi;
    using namespace boost::spirit::ascii;
    using namespace boost::assign;
    using namespace std;

    track_builder builder( instrument_path);

    rythm_grammar_def<std::string::const_iterator> def(
            list_of<string>("do")("ta")("ch")("tu"), builder
        );


    string::const_iterator iter = content.begin();
    string::const_iterator end = content.end();

    bool r = false;

    try
    {
         r = phrase_parse( iter, end, def.file, space);
    }
    catch (std::exception &e)
    {
        std::cerr << "Something went wrong: " << e.what() << std::endl;
    }

    return r;


}