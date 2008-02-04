// RythmBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "note_table.h"
#include "RythmTextGrammar.hpp"

using std::vector;
using std::map;
using std::string;


//
// append the 'right' sound to the 'left' sound
// if one of the producers is null, return the other one.
// if both are null, return null
//
block_producer *append_producer( block_producer *left, block_producer *right)
{
	if (!right)
	{
		return left;
	}

	if (!left)
	{
		return right;
	}

	paste_mutator *paster = new paste_mutator();
	paster->LinkTo( left, 0);
	paster->LinkTo( right, 1);

	return paster;
}

template< typename iterator_t>
block_producer *create_bar_sound( iterator_t current, iterator_t end, note_table *notes)
{
	block_producer *producer = 0;
	while ( current != end)
	{
		producer = append_producer( producer, notes->get_note( *current));
		++current;
	}

	return producer;

}


// spread the sound of the bars over the stereo space (so that the first one is 
// on the utter left channel and the last on the utter right.
//
template<typename iterator_t>
void spread_bars( iterator_t begin, iterator_t end)
{
	if (begin == end) return;

	iterator_t current = begin;
	
	size_t count = std::distance( begin, end);

	stream_header h;
	(*begin)->GetStreamHeader( h);
	sample_object_factory *factory_ptr = factory_factory::GetSampleFactory( h);

	if (count > 1)
	{
		short step = 2 * (32767 / static_cast<short>(count-1));
		short pan_position = -32768;
		while (current != end)
		{
			block_producer *prod_ptr = * current;
			block_mutator *panner = factory_ptr->GetPan( pan_position);
			panner->LinkTo( prod_ptr);
			*current = panner;
			pan_position += step;
			++current;
		}
		current = begin;
	}
}


//
// combine the sound producers for the bars into one sound producer that adds all
// channels
//
template<typename iterator_t>
block_producer *combine_bars( iterator_t begin, iterator_t end)
{
	iterator_t current = begin;
	std::vector<block_producer *> added_producers;
	while (current != end)
	{
		block_producer *prod1 = *current;
		++current;
		if (current != end)
		{
			stream_header h;
			prod1->GetStreamHeader( h);
			sample_object_factory *factory_ptr = factory_factory::GetSampleFactory( h);
			binary_block_processor *adder = factory_ptr->GetAdder();
			delete factory_ptr;
			adder->LinkTo( prod1, *current);
			added_producers.push_back( adder);
			++current;
		}
		else
		{
			added_producers.push_back( prod1);
		}
	}

	if (added_producers.empty())
	{
		return 0;
	}
	else if ( added_producers.size() == 1)
	{
		return added_producers[0];
	}
	else
	{
		return combine_bars( added_producers.begin(), added_producers.end());
	}

}

template< typename container_type>
void add_bar_from_string( container_type &container, const string &bar_string, note_table *table_ptr)
{
	using namespace boost;
	using namespace std;

	tokenizer<> bar_tokenizer( bar_string);
	container.push_back( create_bar_sound( bar_tokenizer.begin(), bar_tokenizer.end(), table_ptr));
}

bool read_rythm_from_textfile( std::vector< std::string> &strings, const char *filename)
{
	std::ifstream infile( filename);
	std::string line;

	if (!infile) return false;
	do 
	{
		std::getline(infile, line);
		if (!line.empty()) strings.push_back( line);
	} while (infile);

	return true;
}

block_producer *resample( block_producer *producer_ptr, short new_sample_rate)
{
	if (!producer_ptr) return 0;

	stream_header h;
	producer_ptr->GetStreamHeader( h);
	sample_object_factory *fact = factory_factory::GetSampleFactory( h);

	block_mutator *resampler_ptr = fact->GetResampler( new_sample_rate, 0, false);
	resampler_ptr->LinkTo( producer_ptr);

	delete fact;

	return resampler_ptr;
}

bool get_commandline_options( int argc, _TCHAR * argv[],
							 std::string &input_file,
							 std::string &output_file,
							 unsigned short &ms_per_note,
							 unsigned short &sample_rate,
							 std::string &note_directory
							 )
{
	--argc;
	++argv;
	if (!argc) return true;

	input_file = *argv;

	--argc;
	++argv;
	if (!argc) 
	{
		output_file = input_file.substr( 0, input_file.find( '.')) + ".wav";
		return true;
	}

	output_file = *argv;

	--argc;
	++argv;
	if (!argc) return true;

	std::istrstream( *argv) >> ms_per_note;

	--argc;
	++argv;
	if (!argc) return true;


	std::istrstream( *argv) >> sample_rate;

	--argc;
	++argv;
	if (!argc) return true;

	note_directory = *argv;

	return true;


}

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace boost;
	using namespace std;
	vector< block_producer *> bars;
	vector< string> strings;

	unsigned short output_sample_rate = 44100;
	string input_file = "rhythm.txt";
	string output_file = "output.wav";
	string note_directory = "default";
	unsigned short ms_per_note = 200;

	get_commandline_options( argc, argv, input_file, output_file, 
		ms_per_note, output_sample_rate, note_directory);
	
	note_table table( note_directory, ms_per_note);


	if (!read_rythm_from_textfile( strings, input_file.c_str()))
	{
		return -1;
	}

	double tune = 0;
	double tune_step = 0;
	if ( strings.size() > 1)
	{
		tune = -2;
		tune_step = 4 / (strings.size() -1);
	}

	vector<string>::iterator i;
	for ( i = strings.begin(); i != strings.end(); ++i)
	{
		table.detune( tune);
		add_bar_from_string( bars, *i, &table);
		tune += tune_step;
	}

	// spread the bars over the stereo space
	spread_bars( bars.begin(), bars.end());

	// now add the sounds of all the bars together
	block_producer *result_ptr = combine_bars( bars.rbegin(), bars.rend());
	result_ptr = resample( result_ptr, output_sample_rate);

	block_sink *pSink = filefactory::GetBlockSink( output_file.c_str());

	if (pSink && result_ptr)
	{
		pSink->LinkTo( result_ptr);
		pSink->Start();
	}
	else
	{
		std::cerr << "could not open either input or output wave files" << std::endl;
	}

	return 0;
}

