// RythmBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "truncator.h"
#include "progress.h"

using std::vector;
using std::map;
using std::string;



bool get_commandline_options( int argc, _TCHAR * argv[],
							 std::string &input_file,
							 std::string &output_file,
							 unsigned short &sample_size,
							 sampleno &cut_start,
							 sampleno &cut_length
							 )
{
	--argc;
	++argv;
	if (!argc) return false;

	input_file = *argv;

	--argc;
	++argv;
	if (!argc) return false;

	output_file = *argv;

	--argc;
	++argv;
	if (!argc) return true;

	std::istrstream( *argv) >> sample_size;

	--argc;
	++argv;
	if (!argc) return true;

	std::istrstream( *argv) >> cut_start;

	--argc;
	++argv;
	if (!argc) return true;

	std::istrstream( *argv) >> cut_length;

	return true;
}

double my_abs( double subject)
{
	if (subject < 0) return -subject;
	else return subject;
}

double get_amplification_factor( block_producer *producer, sample_object_factory *fact)
{
	nil_sink *sink = new nil_sink();
	sample_analyzer *an = fact->GetAnalyzer();

	progress_monitor *monitor = new progress_monitor();
	text_based_progress_bar bar( std::cout, 79);
	monitor->Register( &bar);

	an->LinkTo( producer);
	monitor->LinkTo( an);
	sink->LinkTo( monitor);
	sink->Start();
	sound_analysis result = an->GetResult();

	double maximum = max( my_abs( result.max), my_abs( result.min));

	if (maximum < 0.00000000001)
	{
		return 1.0;
	}
	else
	{
		return 1.0/maximum;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace std;

	string input_file = "input.wav";
	string output_file = "output.wav";
	unsigned short sample_size = 2; // in bytes
	sampleno cut_start = 0, cut_length = 0;

	// get the commandline options
	if (!get_commandline_options( argc, argv, input_file, output_file, 
		sample_size, cut_start, cut_length))
	{
		std::cerr << "usage: truncator <inputfile> <outputfile> [samplesize(bytes)] [cut start (deciseconds)] [cut size (deciseconds)]" 
			<< std::endl;
		return -5;
	}
	

	// open the input file
	block_producer *input = filefactory::GetBlockProducer( input_file.c_str());
	if (!input)
	{
		std::cerr << "could not open input file " << input_file << std::endl;
		return -1;
	}

	stream_header h;
	input->GetStreamHeader( h);
	if (h.samplesize/8 < sample_size)
	{
		std::cerr << "the input file's samplesize is smaller than the target sample size" << std::endl;
		return -4;
	}

	sample_object_factory *fact = factory_factory::GetSampleFactory( h);

	// open the output file
	block_sink *output = filefactory::GetBlockSink( output_file.c_str());
	if (!output)
	{
		std::cerr << "could not open output file " << output_file << std::endl;
		return -2;
	}

	// create the cutter
	cut_mutator cutter;
	if (!cut_length)
	{
		cut_length = (h.numframes / h.samplerate) * 10 - cut_start;
	}
	cutter.LinkTo( input);
	cutter.SetCut( (h.samplerate * cut_start)/10, (h.samplerate * cut_length) / 10);


	// now analyze the samples
	std::cout << "First pass, calculating..." << std::endl;
	double factor = get_amplification_factor( &cutter, fact);

	// build the truncator
	truncator *trunc = new truncator( sample_size, false);
	block_mutator *amplifier = fact->GetAmplifier( factor);

	// create a progress monitor
	progress_monitor *monitor = new progress_monitor();
	text_based_progress_bar bar( std::cout, 79);
	monitor->Register( &bar);

	std::cout << "Writing..." << std::endl;

	// setup the processing chain
	amplifier->LinkTo( &cutter);
	trunc->LinkTo( amplifier);
	monitor->LinkTo( trunc);
	output->LinkTo( monitor);

	// and now start the chain.
	output->Start();

	return 0;
}

