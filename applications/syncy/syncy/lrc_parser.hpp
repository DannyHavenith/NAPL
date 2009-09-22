#if !defined(LRC_PARSER_HPP)
#define LRC_PARSER_HPP
#include <string>
#include <map>

namespace lyrics
{
    typedef std::string line;
    typedef size_t centisecond;
    typedef std::map<centisecond, line> songtext;

    songtext parse_lrc( std::istream &lrc_text);
}

#endif //LRC_PARSER_HPP