#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "samplebl.h"
//#include "samplety.h"
#include "objfact.h"
#include "filefact.h"
#include "simpops.h" // cut and paste mutators

//#include <iostream.h>

int main( int argc, char *argv[])
{
	char *infilename1;
	char *infilename2;
	char *outfilename;

	long deciseconds;

	binary_block_processor *my_xfader;

	// these cutters ar used to cut both input samples to nice pieces: 
	// 1 the start of s1
	// 2 the last n seconds of s1, for xfading with s2
	// 3 the first n seconds of s2 for xfading
	// 4 the tail of s2
	cut_mutator head1;
	cut_mutator tail1; // this one will be combined...
	cut_mutator head2; // ...with this one.
	cut_mutator tail2;

	paste_mutator paste1;
	paste_mutator paste2;

	block_producer *producer1;
	block_producer *producer2;

	block_sink *sink;

	factory_factory ff;
	sample_object_factory *sof;

	stream_header h, h2;

	if (argc != 5)
	{
		puts("usage: xxfade <outfile> <infile> <infile> <fade-length (1/10s)>");
		return -1;
	}

	outfilename = argv[1];
	infilename1 = argv[2];
	infilename2 = argv[3];
	deciseconds = atol( argv[4]);

	printf( "xxfading '%s' and '%s' into '%s'\n", 
		infilename1,
		infilename2,
		outfilename );
	
	producer1 = filefactory::GetBlockProducer( infilename1);
	producer2 = filefactory::GetBlockProducer( infilename2);
	sink = filefactory::GetBlockSink( outfilename);

	if (producer1 && producer2 && sink)
	{
		unsigned long samplesOverlap;

		producer1->GetStreamHeader( h);
		producer2->GetStreamHeader( h2);

		if (h2.numchannels != h.numchannels ||
			h2.samplerate != h.samplerate ||
			h2.samplesize != h.samplesize)
		{
			puts("samples are not compatible");
			return -1;
		}

		head1.LinkTo( producer1);
		tail1.LinkTo( producer1);

		head2.LinkTo( producer2);
		tail2.LinkTo( producer2);

		samplesOverlap = (deciseconds * h.samplerate)/ 10;

		head1.SetCut( 0, h.numframes - samplesOverlap);
		tail1.SetCut( h.numframes - samplesOverlap, samplesOverlap);

		head2.SetCut( 0, samplesOverlap);
		tail2.SetCut( samplesOverlap, h2.numframes - samplesOverlap);

		sof = ff.GetSampleFactory( h);
		if (sof)
		{
			//
			// create the xfader
			//
			my_xfader = sof->GetXFader();
			if (my_xfader)
			{

				// link the xfader to tail1 and head2
				my_xfader->LinkTo( &tail1, &head2);

				// paste head1 and the xfader...
				paste1.LinkTo( &head1, 0);
				paste1.LinkTo( my_xfader, 1);

				// ...and paste tail2 to it
				paste2.LinkTo( &paste1, 0);
				paste2.LinkTo( &tail2, 1);

				sink->LinkTo( &paste2);


				sink->Start();
				delete my_xfader;
			}
			delete sof;
		}
	}
	else puts(" could not open file");
	return 0;
}


