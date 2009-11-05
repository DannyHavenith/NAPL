#if !defined( TEXT_PLAYER_HPP)
#define TEXT_PLAYER_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string/split.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/function.hpp>

#include "lrc_parser.hpp" // for lyrics::songtext and friends
struct text_player_interface
{
    virtual void display( lyrics::centisecond position) = 0;
};

struct text_player : text_player_interface
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

    virtual void display( lyrics::centisecond position)
    {
        while (current != song.end() && current->first <= position)
        {
            emit_lines( *current);
            ++current;
        }
    }

    void prepare_texts( boost::function<void ( lyrics::line &)> adapter)
    {
        for (lyrics::songtext::iterator current = song.begin(); current != song.end(); ++current)
        {
            std::for_each( current->second.begin(), current->second.end(), adapter);
        }
    }

    virtual void end_song() {};
    virtual void emit_lines( const lyrics::line_element &el) = 0;


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
    
    virtual void emit_lines( const lyrics::line_element &el)
    {
        for ( lyrics::lines::const_iterator i = el.second.begin(); i != el.second.end(); ++i)
        {
            std::cout << (int)i->channel << ' ' << i->text << std::endl;
        }
    }
private:
    std::ostream &output;
};

struct composite_textplayer : public text_player_interface
{
    virtual void display( lyrics::centisecond position)
    {
        for (players_vector::iterator i = players.begin(); i != players.end(); ++i)
        {
            (*i)->display( position);
        }
    }

    void add( text_player &player)
    {
        players.push_back( &player);
    }

private:
    typedef std::vector <text_player *> players_vector;
    std::vector <text_player *> players;
};


#endif //TEXT_PLAYER_HPP