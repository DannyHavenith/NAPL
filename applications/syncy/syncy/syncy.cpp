// syncy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>

#include "lrc_parser.hpp"
#include "directsound_player.hpp"
#include "lcd_player.hpp"
#include "mpg_file.hpp" // mp3 file reader

struct lcd_send_options
{
    lcd_send_options()
        :
    serial_device( "COM1"),
    address( 0)
    {
    }

    std::string     serial_device;
    unsigned int    address;
};

struct options : lcd_send_options
{
    std::string sound_filename;
    std::string lrc_filename;
};

/// parse command line options
bool parse_options( int argc, char *argv[], options &opts)
{
        namespace po=boost::program_options;

        // reset options to defaults
        opts = options();

        // parse program options
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help",        
                "produce this help message")
                ("address,a",   po::value<unsigned int>(&opts.address), 
                "receiver address to send to")
                ("port,p",      po::value< std::string>(&opts.serial_device), 
                "serial device")
            ;

        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("lrc-file", po::value< std::string>(&opts.lrc_filename), "lyrics file")
            ("sound-file", po::value< std::string>(&opts.sound_filename), "wave file");

        po::positional_options_description p;
        p.add("lrc-file", 1);
        p.add("sound-file", 1);

        po::options_description all("All options");
        all.add( desc).add(hidden);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
            options(all).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help") || !vm.count("lrc-file")) 
        {
            std::cout << "usage: syncy lrc-file [sound-file] <options>\n";
            std::cout << desc << "\n";
            return false;
        }

        if (!vm.count("sound-file"))
        {
            using namespace boost::filesystem;

            path p( opts.lrc_filename);
            opts.sound_filename = p.replace_extension(".mp3").string();
        }
    return true;
}

int main(int argc, char* argv[])
{
    // this application needs COM to access directsound
    HRESULT hr = ::CoInitialize(NULL);


    if(FAILED(hr))
    {
        std::cerr << "Could not initialize COM for DirectSound" << std::endl;
        return -1;
    }

    options opts;
    if (! parse_options( argc, argv, opts)) 
    {
        return -2;
    }

    try
    {
        directsound_wrapper ds;

        // open an mp3 sound producer.
        mp3_block_producer mp3_file( opts.sound_filename.c_str());


        // try to open the lyrics file and parse it to obtain text records.
        std::ifstream lyricsfile( opts.lrc_filename.c_str());
        if (!lyricsfile) throw std::runtime_error( "could not open lyrics (lrc-)file" + opts.lrc_filename);
        lyrics::songtext text = lyrics::parse_lrc( lyricsfile, lyrics::numeric_channel_converter( opts.address));

        // set up both an lcd player and a console player. wrap them in a composite.
        lcd_player lcd( text, opts.serial_device, opts.address);
        console_textplayer console( text, std::cout);
        composite_textplayer text_players;
        text_players.add( lcd);
        text_players.add( console);

        stream_header h;
        mp3_file.GetStreamHeader( h);

        // set up the sound player.
        directsound_player player = ds.create_player( h);

        // couple it to the file source
        player.LinkTo( &mp3_file);

        // send timing events to the composite player
        player.register_position_handler( boost::bind( &text_player_interface::display, &text_players, _1));
        player.start();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ::CoUninitialize();
    return 0;
}

