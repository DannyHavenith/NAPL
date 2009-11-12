#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

// napl midi file parser
#include "midi/midi_parser.hpp"
#include "midi/midi_event_visitor.hpp"
#include "midi/midi_multiplexer.hpp"
#include "midi/timed_midi_visitor.hpp"

#include "midi.hpp"
#include "lrc_parser.hpp"

struct print_text_visitor : public events::timed_visitor<print_text_visitor>
{
	using events::timed_visitor<print_text_visitor>::operator();
	typedef events::timed_visitor< print_text_visitor> parent;

	print_text_visitor( midi_header &h)
		: parent( h)
	{
	}

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

///
/// This class extracts text events from a midi-file and adds the text to a songtext object.
///
struct extract_text_visitor: public events::timed_visitor<extract_text_visitor>
{
	typedef events::timed_visitor< print_text_visitor> parent;
	using parent::operator();

	extract_text_visitor( midi_header &h)
		: parent( h)
	{
	}

	void operator()( const events::meta &event)
	{
		// is it a text event?
		if (event.type == 0x05 || (event.type == 0x01))
		{
			// then add it to the songtext
			text[current_time/100].push_back( std::string( event.bytes.begin(), event.bytes.end()));
		}
		else
		{
			// else let our parent handle it...
			parent::operator()( event);
		}
	}

	lyrics::songtext get_text() const
	{
		return text;
	}

private:
	lyrics::songtext text;
};

lyrics::songtext parse_midi_text( const boost::filesystem::path &p)
{
	using namespace std;

	ifstream in(p.string(), ios::binary);

	if (!in)
	{
		throw std::runtime_error( "could not open file " + p.string());
	}
	midi_file contents;
	if (!parse_midifile( in, contents))
	{
	}
	else
	{
		typedef std::vector< midi_track>::iterator track_iterator;
		for (track_iterator i = contents.tracks.begin(); i != contents.tracks.end(); ++i)
		{
			print_text_visitor v( contents.header);
			std::for_each( i->begin(), i->end(), v);
		}

		midi_track track;
		midi_multiplexer multiplexer( contents.tracks);
		multiplexer.accept( print_text_visitor(contents.header));
//		multiplexer.accept( multi_to_single_track( track));
	}


}

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
			print_text_visitor v( contents.header);
			std::for_each( i->begin(), i->end(), v);
		}

		midi_track track;
		midi_multiplexer multiplexer( contents.tracks);
		multiplexer.accept( print_text_visitor(contents.header));
//		multiplexer.accept( multi_to_single_track( track));
	}


	return 0;
}

