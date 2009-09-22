// syncy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <boost/bind.hpp>
#include "lrc_parser.hpp"
#include "directsound_player.hpp"

void testfunc( size_t pos)
{
    if (pos%100 == 0)
    {
        std::cout << '.';
    }
};

struct textplayer
{
    textplayer( const lyrics::songtext &song_, std::ostream &output_)
        :song(song_), output( output_)
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
            output << current->second << std::endl;
            ++current;
        }
    }

    lyrics::songtext song;
    lyrics::songtext::const_iterator current;
    std::ostream &output;
};

int main(int argc, char* argv[])
{
    // this application needs COM to access directsound
    HRESULT hr = ::CoInitialize(NULL);

    if(FAILED(hr))
    {
        return -1;
    }

    try
    {
        directsound_wrapper ds;
        const size_t buffer_size = 441000 * 2 * 2;
        block_producer *p = filefactory::GetBlockProducer( argv[1]);
        std::ifstream lyricsfile( argv[2]);
        lyrics::songtext text = lyrics::parse_lrc( lyricsfile);
        textplayer player( text, std::cout);

        stream_header h;
        p->GetStreamHeader( h);
        directsound_player buffer = ds.create_player( h);
        buffer.LinkTo( p);
        buffer.register_position_handler( boost::bind( &textplayer::display, &player, _1));
        buffer.start();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ::CoUninitialize();
    return 0;
}

