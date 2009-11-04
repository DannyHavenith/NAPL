#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

// napl midi file parser
#include "midi/midi_parser.hpp"
#include "midi/midi_event_visitor.hpp"


struct print_text_visitor : public events::timed_visitor<print_text_visitor>
{
	using events::timed_visitor<print_text_visitor>::operator();

	void operator()( const events::note_on &note)
	{
		std::cout << "n:" << static_cast<int>( note.number);
	}

	void operator()( const events::note_off &note)
	{
		std::cout << "o:" << static_cast<int>( note.number);
	}

	void operator()( const events::meta &event)
	{
		if (event.type == 0x05 || (event.type == 0x01))
		{
			std::cout << current_time << '\t' << std::string( event.bytes.begin(), event.bytes.end()) << '\n';
		}
	}
};


int midi_testfunc(int argc, char* argv[])
{
	using namespace std;

	if (argc < 2)
	{
		cerr << "usage: parsemidi <filename>\n";
		return -1;
	}

	ifstream in(argv[1], ios::binary);

	if (!in)
	{
		cerr << "could not open " << argv[1] << '\n';
		return -2;
	}

	midi_file contents;
	if (parse_midifile( in, contents))
	{
		typedef std::vector< midi_track>::iterator track_iterator;
		for (track_iterator i = contents.tracks.begin(); i != contents.tracks.end(); ++i)
		{
			print_text_visitor v;
			std::for_each( i->begin(), i->end(), v);
		}

	}


	return 0;
}

