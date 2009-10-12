#include "stdafx.h"
#include <boost/regex.hpp>
#include "lrc_parser.hpp"
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>

using namespace std;
using namespace boost::xpressive;

namespace lyrics
{
    songtext parse_lrc( istream &lrc_text, const channel_converter &converter)
    {
        int minute = 0;
        int second = 0;
        int csecond = 0;
        string channel_text;
        string line_text;

        const sregex time = (s1 = repeat<2>( digit)) >> ':' >> (s2 = repeat<2>(digit)) >> '.' >> (s3 = repeat<2>(digit)) 
            [ref(minute)=as<int>(s1), ref(second) = as<int>(s2), ref(csecond) = as<int>(s3)];
        const sregex ignore_time = (s1 = repeat<2>( digit)) >> ':' >> (s2 = repeat<2>(digit)) >> '.' >> (s3 = repeat<2>(digit));

        const sregex line_tag = '[' >> time >> ']';
        const sregex word_tag = '<' >> ignore_time >> '>';
        const sregex channel_tag = ':' >> (s1 = +digit)[ ref(channel_text) = s1];

        const sregex line = 
                line_tag >> optional(channel_tag) >> *(!word_tag >> (s1 = (-*_)))
                [ref(line_text) += s1]
            ;
        
        string buffer;
        smatch match;
        songtext result;
        while (getline( lrc_text, buffer))
        {
            if (!buffer.empty() && regex_match( buffer, match, line))
            {
                centisecond time = ((minute * 60) + second) * 100 + csecond;
                result[time].push_back( 
                    lyrics::line( 
                        converter.convert( channel_text), 
                        line_text
                    )
                );
            }

            line_text.clear();
            channel_text.clear();
        }
        return result;
    }
}
