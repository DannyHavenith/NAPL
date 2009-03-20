#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "samplebl.h"
#include "samplety.h"
#include "objfact.h"
#include "filefact.h"

#include <iostream.h>

int main( int argc, char *argv[])
{
	char *infilename1;
	char *infilename2;
	char *outfilename;

	binary_block_processor *my_xfader;

	block_producer *producer1;
	block_producer *producer2;

	block_sink *sink;

	factory_factory ff;

	stream_header h;
	sample_object_factory *sof;

	if (argc != 4)
	{
		puts("usage: xfade <outfile> <infile> <infile>");
		return -1;
	}

	outfilename = argv[1];
	infilename1 = argv[2];
	infilename2 = argv[3];

	printf( "xfading '%s' and '%s' into '%s'\n", 
		infilename1,
		infilename2,
		outfilename );
	
	producer1 = filefactory::GetBlockProducer( infilename1);
	producer2 = filefactory::GetBlockProducer( infilename2);
	sink = filefactory::GetBlockSink( outfilename);

	if (producer1 && producer2 && sink)
	{
		producer1->GetStreamHeader( h);

		sof = ff.GetSampleFactory( h);
		if (sof)
		{
			my_xfader = sof->GetXFader();
			if (my_xfader)
			{

				my_xfader->LinkTo( producer1, producer2);
				sink->LinkTo( my_xfader);


				sink->Start();
				delete my_xfader;
			}
			delete sof;
		}
	}
	else puts(" could not open file");
	return 0;
}


