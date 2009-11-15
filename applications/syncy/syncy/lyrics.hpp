/// This file defines the lyrics datastructure.

#if !defined( LYRICS_HPP)
#define LYRICS_HPP

namespace lyrics
{
    /// This class represents a piece of text that needs to be sent
    /// to a specific channel.
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

    /// one hundredth of a second
    typedef size_t centisecond;
    typedef std::vector<line> lines;

    /// a songtext is a mapping of a time position to a number of lines.
    typedef std::map<centisecond, lines > songtext;
    typedef songtext::value_type line_element;
} // end namespace lyrics

#endif //LYRICS_HPP