#if !defined(LRC_PARSER_HPP)
#define LRC_PARSER_HPP
#include <string>
#include <map>
#include <vector>
#include <boost/lexical_cast.hpp>

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

    struct line
    {
        explicit line( const std::string &text_)
            : channel(0), text( text_)
        {
        }

        line( unsigned char channel_, const std::string &text_)
            : channel( channel_), text( text_)
        {
        }

        std::string text;
        unsigned char channel;
    };

    typedef size_t centisecond;
    typedef std::vector<line> lines;
    typedef std::map<centisecond, lines > songtext;
    typedef songtext::value_type line_element;

    /// parse an lrc file and generate a songtext structure.
    songtext parse_lrc( 
        std::istream &lrc_text, 
        const channel_converter &converter = numeric_channel_converter(0));
}

#endif //LRC_PARSER_HPP