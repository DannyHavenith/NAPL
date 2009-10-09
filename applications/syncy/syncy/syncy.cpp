// syncy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "lrc_parser.hpp"
#include "directsound_player.hpp"
#include "lcd_player.hpp"
#include "mpg_file.hpp" // mp3 file reader

void testfunc( size_t pos)
{
    if (pos%100 == 0)
    {
        std::cout << '.';
    }
};


int main(int argc, char* argv[])
{
    // this application needs COM to access directsound
    HRESULT hr = ::CoInitialize(NULL);

    if(FAILED(hr))
    {
        std::cerr << "Could not initialize COM for DirectSound" << std::endl;
        return -1;
    }

    try
    {
        namespace po=boost::program_options;

        unsigned int address = 0;
        std::string port = "COM1";
        std::string mp3_filename;
        std::string lrc_filename;

        // parse program options
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help",        
                "produce this help message")
            ("address,a",   po::value<unsigned int>(&address), 
                "receiver address to send to")
            ("port,p",      po::value< std::string>(&port), 
                "serial device")
            ;

        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("lrc-file", po::value< std::string>(&lrc_filename), "lyrics file")
            ("mp3-file", po::value< std::string>(&mp3_filename), "wave file");

        po::positional_options_description p;
        p.add("lrc-file", 1);
        p.add("mp3-file", 1);

        po::options_description all("All options");
        all.add( desc).add(hidden);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
            options(all).positional(p).run(), vm);
        po::notify(vm);

        if (vm.count("help") || !vm.count("lrc-file")) 
        {
            std::cout << "usage: syncy lrc-file [mp3-file] <options>\n";
            std::cout << desc << "\n";
            return 1;
        }

        if (!vm.count("mp3-file"))
        {
            using namespace boost::filesystem;

            path p( lrc_filename);
            mp3_filename = p.replace_extension(".mp3").string();
        }


        directsound_wrapper ds;
        mp3_block_producer mp3_file( mp3_filename.c_str());
//        if (!producer) throw std::runtime_error("could not open mp3 file " + mp3_filename);

        std::ifstream lyricsfile( lrc_filename.c_str());
        if (!lyricsfile) throw std::runtime_error( "could not open lyrics (lrc-)file" + lrc_filename);

        lyrics::songtext text = lyrics::parse_lrc( lyricsfile);

        lcd_player player( text, port, address);
//        console_textplayer player( text, std::cout);

        stream_header h;
        mp3_file.GetStreamHeader( h);
        directsound_player buffer = ds.create_player( h);
        buffer.LinkTo( &mp3_file);
        buffer.register_position_handler( boost::bind( &text_player::display, &player, _1));
        buffer.start();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ::CoUninitialize();
    return 0;
}

