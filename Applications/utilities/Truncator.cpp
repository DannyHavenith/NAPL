// RythmBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "truncator.h"

using std::vector;
using std::map;
using std::string;



bool get_commandline_options( int argc, _TCHAR * argv[],
							 std::string &input_file,
							 std::string &output_file,
							 unsigned short &sample_size
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

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace boost;
	using namespace std;
	vector< block_producer *> bars;
	vector< string> strings;

	string input_file = "input.wav";
	string output_file = "output.wav";
	unsigned short sample_size = 2; // in bytes

	if (!get_commandline_options( argc, argv, input_file, output_file, 
		sample_size))
	{
		std::cerr << "usage: truncator <inputfile> <outputfile> <samplesize(bytes)>" 
			<< std::endl;
	}
	

	block_producer *input = filefactory::GetBlockProducer( input_file.c_str());
	if (!input)
	{
		std::cerr << "could not open input file " << input_file << std::endl;
		return -1;
	}

	stream_header h;
	input->GetStreamHeader( h);
	if (h.samplesize/8 <= sample_size)
	{
		std::cerr << "the input file's samplesize is smaller than the target sample size" << std::endl;
		return -4;
	}

	block_sink *output = filefactory::GetBlockSink( output_file.c_str());
	if (!output)
	{
		std::cerr << "could not open output file " << output_file << std::endl;
		return -2;
	}

	truncator *trunc = new truncator( sample_size, false);

	// setup the processing chain
	trunc->LinkTo( input);
	output->LinkTo( trunc);

	try
	{
		output->Start();
	}
	catch (truncator::truncation_exception &)
	{
		std::cerr << "operation cancelled because truncated bytes were not zero" << std::endl;
		return -3;
	}

	std::cout << "skip range [" << trunc->get_min() << "," << trunc->get_max() << "]" << std::endl;

	return 0;
}

