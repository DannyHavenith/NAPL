#include "stdafx.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <iostream>
using namespace std;

int main( int argc, char *argv[])
{
	char *infilename;
	char *outfilename;

	long deciseconds;
	long length;

	cut_mutator cutter;

	block_producer *producer;
	block_sink *sink;

	stream_header h;

	if (argc != 5)
	{
		puts("usage: cut <outfile> <infile> <start> <length>\n"
			 "both start and length are in 1/10 seconds");
		return -1;
	}

	outfilename = argv[1];
	infilename = argv[2];

	deciseconds = atol(argv[3]);
	length = atol(argv[4]);

	printf( "cutting '%s' at decisecond %ld, length = %ld dsecs, output = '%s'\n",
		infilename, deciseconds, length, outfilename);

	
	producer = filefactory::GetBlockProducer( infilename);
	sink = filefactory::GetBlockSink( outfilename);

	if (producer && sink)
	{
		producer->GetStreamHeader( h);

		cutter.LinkTo( producer);
		sink->LinkTo( &cutter);
		cutter.SetCut( (h.samplerate * deciseconds)/10, (h.samplerate * length) / 10);
		sink->Start();
	}
	else puts(" could not open file");
	return 0;
}


