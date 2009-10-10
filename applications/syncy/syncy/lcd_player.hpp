#if !defined (LCD_PLAYER_HPP)
#define LCD_PLAYER_HPP
#include <string>
#include "text_player.hpp"
#include "lcd_send/serial_port_sink.hpp"
#include "lcd_send/packager.hpp"
#include "lcd_send/translate_strings.hpp"

struct lcd_player: text_player
{
    lcd_player( lyrics::songtext &song, const std::string &port, unsigned int address)
        :text_player( song), sink(port), pack( sink, address)
    {
        // adapt the texts to fit in the lcd display.
        prepare_texts( &lcd_player::adapt_text);
    }

    virtual void emit_line( const lyrics::line_element &el)
    {
        std::string text( el.second);
        pack.send_string( text);
        std::cout << el.second << std::endl;
    }

    virtual void end_song()
    {
        std::string end = "<cls>";
        translate_special_inplace( end);
        pack.send_string( end);
    }

    //
    /// Adapt text to fit this particular player.
    /// This implementation word-wraps the text to the number of lines 
    /// of the display.
    static void adapt_text( std::string &text)
    {
        typedef std::vector<std::string> stringvector;
        using namespace boost::algorithm;

        stringvector words;
        split( words, text, is_space());

        stringvector::const_iterator current_word = words.begin();

        if ( current_word != words.end())
        {
            std::string buffer = "<cls>" + *current_word;
            int column = current_word->size();
            int line = 0;
            ++current_word;
            while (current_word != words.end() && line != lines)
            {
                int new_column = column + current_word->size() + 1;

                // yes, that's '>', not '>='
                if (new_column > cols)
                {
                    // the word does not fit on the current line, move to a new line
                    ++line;
                    buffer += "<goto 0 " + boost::lexical_cast< std::string>( line) + '>' + *current_word;
                    column = current_word->size();
                }
                else
                {
                    // the word fits on the current line
                    buffer += " " + *current_word;
                    column = new_column;
                }
                ++current_word;
            }
            translate_special_inplace( buffer);
            text = buffer;
        }
    }

private:
    static const int lines = 4;
    static const  int cols = 20;
    serial_port_sink sink;
    packager pack;
};

#endif //LCD_PLAYER_HPP