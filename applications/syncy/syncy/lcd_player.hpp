#if !defined (LCD_PLAYER_HPP)
#define LCD_PLAYER_HPP
#include <string>
#include "text_player.hpp"
#include "lcd_send/serial_port_sink.hpp"
#include "lcd_send/packager.hpp"
#include "lcd_send/translate_strings.hpp"

struct text_compactor
{

};

struct lcd_player: text_player
{
    lcd_player( lyrics::songtext &song, const std::string &port, unsigned int address)
        :text_player( song), sink(port), pack( sink, address)
    {
        // adapt the texts to fit in the lcd display.
        prepare_texts( &lcd_player::adapt_line);
    }

    virtual void emit_lines( const lyrics::line_element &el)
    {
        for ( lyrics::lines::const_iterator i = el.second.begin(); i != el.second.end(); ++i)
        {
            pack.send_string( i->text, i->channel);
        }
    }

    /// try to fit more lines in a single display.
    static lyrics::songtext compact_songtext( const lyrics::songtext &original_song)
    {
        using namespace lyrics;
        songtext song;
        for (songtext::const_iterator i = original_song.begin(); i != original_song.end(); ++i)
        {
            for (lines::const_iterator l = i->second.begin(); l != i->second.end(); ++l)
            {
                
            }
        }
    }

    virtual void end_song()
    {
        std::string end = "<cls>";
        translate_special_inplace( end);
        pack.send_string( end);
    }

    static void adapt_line( lyrics::line &line)
    {
        adapt_text( line.text);
    }

    //
    /// Adapt text to fit this particular player.
    /// This implementation word-wraps the text to the number of lines 
    /// of the display.
    static int adapt_text( std::string &text, int starting_line = 0)
    {
        typedef std::vector<std::string> stringvector;
        using namespace boost::algorithm;
        int line = starting_line;

        stringvector words;

        // split the string on spaces. 
        split( words, text, is_space(), token_compress_on);
        stringvector::const_iterator current_word = words.begin();

        if ( current_word != words.end())
        {
            std::string buffer = starting_line? ("<goto 0 " + boost::lexical_cast< std::string>( line) + '>'):"<cls>";
            buffer += *current_word;
            int column = current_word->size();
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
        return line;
    }

private:
    static const int lines = 4;
    static const  int cols = 20;
    serial_port_sink sink;
    packager pack;
};

#endif //LCD_PLAYER_HPP