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
#include "midi.hpp"
#include "midi_player.hpp"

namespace fs = boost::filesystem;

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
	fs::path sound_filename;
	fs::path midi_filename;
	fs::path lrc_filename;
};

/// parse command line options
bool parse_options( int argc, char *argv[], options &opts)
{
	namespace po = boost::program_options;

    typedef std::vector< std::string> file_vector;
	file_vector filenames;

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
        ("file", po::value< std::vector< std::string> >(&filenames), "mp3, midi or lrc-file");
	//&opts.sound_filename
	po::positional_options_description p;
	p.add("file", -1);

	po::options_description all("All options");
	all.add( desc).add(hidden);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).
		options(all).positional(p).run(), vm);
	po::notify(vm);


	// based on the file extension, allocate command line arguments to 
	// different types of filenames.
	for (file_vector::const_iterator i = filenames.begin(); i != filenames.end(); ++i)
	{
        fs::path p( *i);
		const std::string extension = boost::algorithm::to_lower_copy( p.extension());
		if (extension == ".mp3")
		{
			opts.sound_filename = p;
		}
		else if (extension == ".mid" || extension == ".kar")
		{
			opts.midi_filename = p;
		}
		else if (extension == ".lrc")
		{
			opts.lrc_filename = p;
		}
	}

	// we need a sound file. If none was given, try to determine the name of the 
	// sound file from a given midi- or lrc file path.
	if (opts.sound_filename.empty())
	{
		if (!opts.lrc_filename.empty())
		{
			opts.sound_filename = opts.lrc_filename;
			opts.sound_filename.replace_extension( ".mp3");
		}
		else if (!opts.midi_filename.empty())
		{
			opts.sound_filename = opts.midi_filename;
			opts.sound_filename.replace_extension( ".mp3");
		}
	}

	// if we still don't have a path to a sound file, we return an error result.
	if (opts.sound_filename.empty()) 
	{
		std::cout << "usage: syncy [lrc-file|sound-file|midi-file]... <options>\n";
		std::cout << desc << "\n";
		return false;
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


	try
	{
		directsound_wrapper ds;

	    options opts;
	    if (! parse_options( argc, argv, opts)) 
	    {
		    return -2;
	    }

        // open an mp3 sound producer.
		mp3_block_producer mp3_file( opts.sound_filename.string().c_str());


		lyrics::songtext text;

        bool have_text = false;
		if (!opts.lrc_filename.empty())
		{
			// try to open the lyrics file and parse it to obtain text records.
			std::ifstream lyricsfile( opts.lrc_filename.string().c_str());
			if (!lyricsfile) throw std::runtime_error( "could not open lyrics (lrc-)file" + opts.lrc_filename.string());
			text = lyrics::parse_lrc( lyricsfile, lyrics::numeric_channel_converter( opts.address));
            have_text = true;
		}
		else if (!opts.midi_filename.empty())
		{
			text = parse_midi_text( opts.midi_filename, opts.address);
            have_text = !text.empty();
		}

		// set up both an lcd player and a console player. wrap them in a composite.
		lcd_player lcd( text, opts.serial_device, opts.address);
		console_textplayer console( text, std::cout);
        composite_textplayer text_players;

        if (have_text)
        {
    		text_players.add( lcd);
	    	text_players.add( console);
        }

        // if there's a midi filename, add a midi player to the composite
        midi_player midi;        
        if (!opts.midi_filename.empty())
        {
            midi_player_from_file( opts.midi_filename, midi);
            text_players.add( midi);
        }

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

