// syncy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "lrc_parser.hpp"
#include "directsound_player.hpp"
#include "objfact.h" // napl object factory
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
    options()
        :midi_delay(0.0)
    {
    }

	fs::path sound_filename;
	fs::path midi_filename;
	fs::path lrc_filename;
    double midi_delay;
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
        ("delay,d", po::value<double>( &opts.midi_delay),
        "midi delay in seconds")
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
		if (extension == ".mp3" || extension == ".wav")
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
	// sound file from a given lrc file path.
	if (
            opts.sound_filename.empty()
        &&  opts.midi_filename.empty()
        &&  !opts.lrc_filename.empty())
	{
        fs::path p = opts.lrc_filename;
        if (fs::exists( p.replace_extension("mp3")))
        {
            opts.sound_filename = p;
        }
        else if (fs::exists( p.replace_extension("wav")))
        {
            opts.sound_filename = p;
        }
        else if (fs::exists( p.replace_extension("mid")))
        {
            opts.midi_filename = p;
        }
        else if (fs::exists( p.replace_extension( "kar")))
        {
            opts.midi_filename = p;
        }
	}

	// if we still don't have a path to a sound file, we return an error result.
	if (opts.sound_filename.empty() && opts.midi_filename.empty()) 
	{
		std::cout << "usage: syncy [lrc-file|sound-file|midi-file]... <options>\n";
		std::cout << desc << "\n";
		return false;
	}

	return true;
}

block_producer *create_wave_producer( const fs::path &p)
{
    block_producer *result = 0;
    if (boost::algorithm::to_lower_copy(p.extension()) == ".mp3")
    {
        result = new mp3_block_producer( p.string().c_str());
    }
    else
    {
        result = filefactory::GetBlockProducer( p.string());
    }

    return result;
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


		lyrics::songtext text;

        // see if we can retrieve text from either an lrc file or a midi (or karaoke) file.
        bool there_is_text = false;
		if (!opts.lrc_filename.empty())
		{
			// try to open the lyrics file and parse it to obtain text records.
			std::ifstream lyricsfile( opts.lrc_filename.string().c_str());
			if (!lyricsfile) throw std::runtime_error( "could not open lyrics (lrc-)file " + opts.lrc_filename.string());
			text = lyrics::parse_lrc( lyricsfile, lyrics::numeric_channel_converter( opts.address));
            there_is_text = true;
		}
		else if (!opts.midi_filename.empty())
		{
			text = parse_midi_text( opts.midi_filename, opts.address);
            there_is_text = !text.empty();
		}

		// set up both an lcd player and a console player. wrap them in a composite.
        // We're using a scoped ptr here so that we can construct in a try block.
        // this is needed to give a little more information than the "file not found" 
        // message you get on windows when there's issues with the serial port.
        boost::scoped_ptr< lcd_player> lcd;
        try
        {
            lcd.reset( new lcd_player(text, opts.serial_device, opts.address));
        }
        catch (std::exception &)
        {
            std::cerr << "Error while opening serial device\n";
            throw;
        }

		console_textplayer console( text, std::cout);
        composite_textplayer text_players;

        if (there_is_text)
        {
    		text_players.add( *lcd);
	    	text_players.add( console);
        }

        // if there's a midi filename, add a midi player to the composite
        midi_player midi;        
        if (!opts.midi_filename.empty())
        {
            midi_player_from_file( opts.midi_filename, midi, opts.midi_delay);
            text_players.add( midi);
        }

        // if an mp3 filename was given, open it. Otherwise create a silence that is long enough for the midi 
        // file
        boost::scoped_ptr< block_producer> wave_sound;
		stream_header h;
        if (!opts.sound_filename.empty())
        {
            wave_sound.reset( create_wave_producer( opts.sound_filename));
            wave_sound->GetStreamHeader( h);
        }
        else
        {
            double seconds = midi.length();
            h.samplerate = 44100;
            h.samplesize = 16;
            h.numchannels = 2;
            h.numframes = static_cast< unsigned long>( seconds * 44100);
            h.architecture = LOCAL_ARCHITECTURE; // local endianness
            sample_object_factory *f =
                factory_factory::GetSampleFactory( h);
            wave_sound.reset( f->GetConstant( h));
        }

		// set up the sound player.
		directsound_player player = ds.create_player( h);

		// couple it to the file source
		player.LinkTo( wave_sound.get());

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