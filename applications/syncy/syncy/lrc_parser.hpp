#if !defined(LRC_PARSER_HPP)
#define LRC_PARSER_HPP
#include <string>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>
#include "lyrics.hpp"

namespace lyrics
{

    struct channel_converter
    {
        virtual unsigned int convert( const std::string &channel_text) const = 0;
        virtual ~channel_converter(){};
    };


    /// simple implementation of a channel converter, just tries to translate
    /// text to it's numeric equivalent (e.g. "1" to 1).
    struct numeric_channel_converter : channel_converter
    {
        explicit numeric_channel_converter( unsigned int default_channel_ = 0)
            : default_channel( default_channel_)
        {
        }

        virtual unsigned int convert (const std::string &channel_text) const
        {
            int result = default_channel;

            // emtpy string means default channel, this is more of a performance optimization.
            if (!channel_text.empty())
            {
                // otherwise, try to convert to a number
                try
                {
                    result = boost::lexical_cast< unsigned int>( channel_text);
                }
                catch( boost::bad_lexical_cast &)
                {
                    // do noting, default result will be returned.
                }
            }
            return result;
        }
    private:
        const unsigned int default_channel;
    };

    /// parse an lrc file and generate a songtext structure.
    songtext parse_lrc( 
        std::istream &lrc_text, 
        const channel_converter &converter = numeric_channel_converter(0));
}

#endif //LRC_PARSER_HPP