#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include <boost/algorithm/string/predicate.hpp> // for starts_with, ends_with

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
/// This class extracts text events from a midi-file and adds the text to a songtext 
/// object at the right instances.
///
struct extract_text_visitor: public events::timed_visitor<extract_text_visitor>
{
	typedef events::timed_visitor< extract_text_visitor> parent;
	using parent::operator();

    extract_text_visitor( midi_header &h, lyrics::songtext &text_, int channel_)
		: parent( h), text( text_), channel( channel_)
	{
	}


    ~extract_text_visitor()
    {
        flush();
    }

	void operator()( const events::meta &event)
	{
        using namespace boost::algorithm;
		// is it a text event?
        // we're ignoring lyrics (0x05) events, because text events have more
        // information (like 'start of new line')
		if (event.type == 0x01)
		{
            std::string event_text( event.bytes.begin(), event.bytes.end());
            if (event_text.size() > 0 && event_text[0] != '@')
            {
                if (event_text[0] == '/' || event_text[0] == '\\')
                {
                    flush();
                    add_to_current_line( event_text.substr( 1));
                }
                else
                {
                    add_to_current_line( event_text);
                }
            }
		}
		else
		{
			// else let our parent handle it...
			parent::operator()( event);
		}
	}

    void flush()
    {
        if (!current_line.empty())
        {
            // then add it to the songtext.
            // songtext works in centiseconds resolution.
            text[static_cast<unsigned int>(current_line_time*100)]
                .push_back( 
                    lyrics::line( current_line)
                    );
            current_line.clear();
        }
    }

    void add_to_current_line( const std::string &word)
    {
        if (current_line.empty())
        {
            current_line_time = current_time;
        }

        current_line += word;
    }

private:
    double              current_line_time;
    std::string         current_line;
    const unsigned int  channel;
	lyrics::songtext    &text;
};

lyrics::songtext parse_midi_text( const boost::filesystem::path &p, unsigned int channel)
{
	using namespace std;
    lyrics::songtext result;

	ifstream in(p.string().c_str(), ios::binary);

	if (!in)
	{
		throw std::runtime_error( "could not open file " + p.string());
	}

	midi_file contents;
	if (!parse_midifile( in, contents))
	{
        throw std::runtime_error( "corrupt midi file " + p.string());
	}
	else
	{

        // merge all tracks into one track
		midi_multiplexer multiplexer( contents.tracks);

        // and then offer all events to the text extractor.
		multiplexer.accept( extract_text_visitor(contents.header, result, channel));
	}

    return result;
}

