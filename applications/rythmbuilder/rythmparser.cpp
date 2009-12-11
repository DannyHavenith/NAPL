#include "stdafx.h"
#include <boost/assign.hpp>
#include <boost/spirit/include/qi.hpp>

#include "rythmparser.h"
#include "track_builder.h"
#include "rythmtextgrammar.hpp"

bool ParseRythm(  const boost::filesystem::path &instrument_path, 
                const std::string &content,
                const std::string &default_track_name)
{
    using namespace boost::spirit;
    using namespace boost::spirit::qi;
    using namespace boost::spirit::ascii;
    using namespace boost::assign;
    using namespace std;

    instrument_factory f( instrument_path);

    track_builder builder( f, default_track_name);

    rythm_grammar_def<std::string::const_iterator> def( f.get_note_names(), builder);


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
