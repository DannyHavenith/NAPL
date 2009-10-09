#if !defined( TEXT_PLAYER_HPP)
#define TEXT_PLAYER_HPP

#include <string>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>

struct text_player
{
    text_player( const lyrics::songtext &song_)
        :song(song_)
    {
        reset();
    }

    void reset()
    {
        current = song.begin();
    }

    void display( lyrics::centisecond position)
    {
        while (current != song.end() && current->first <= position)
        {
            emit_line( *current);
            ++current;
        }
    }

    void prepare_texts( boost::function<void ( std::string &)> adapter)
    {
        for (lyrics::songtext::iterator current = song.begin(); current != song.end(); ++current)
        {
            adapter( current->second);
        }
    }

    virtual void end_song() {};

protected:
    virtual void emit_line( const lyrics::line_element &el) = 0;

private:
    lyrics::songtext song;
    lyrics::songtext::const_iterator current;
};

struct console_textplayer : text_player
{
    console_textplayer( const lyrics::songtext &song, std::ostream &output_)
        : text_player( song), output( output_)
    {
    }
    
    virtual void emit_line( const lyrics::line_element &el)
    {
        output << el.second << std::endl;
    }
private:
    std::ostream &output;
};


#endif //TEXT_PLAYER_HPP