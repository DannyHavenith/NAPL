#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "samplebl.h"
#include "samplety.h"
#include "resample.h"
#include "objfact.h"
#include "filefact.h"

//#include "convendi.h" // endian converters
#include <iostream.h>

int main( int argc, char *argv[])
{
	char *infilename;
	char *outfilename;
	unsigned short outfreq;
	unsigned short infreq;
	bool lie;
	block_producer *producer;
	block_sink *sink;

	factory_factory ff;

	stream_header h;
	sample_object_factory *sof;

	if (argc != 6)
	{
		puts("usage: resample <outfile> <infile> <outfreq> <infreq( 0 = original)> <lie:(y/n)>");
		return -1;
	}

	outfilename = argv[1];
	infilename = argv[2];

	outfreq = (unsigned short)atol(argv[3]);
	infreq = (unsigned short)atol(argv[4]);

	lie = (argv[5][0] == 'Y' || argv[5][0] == 'y');

	printf("resampling\nfile: %s<-%s\nfreq: %ud<-%ud\nlie = %c\n",
		outfilename, infilename,
		(unsigned int)outfreq, (unsigned int)infreq,
		lie?'Y':'N'
		);

	general_resampler *my_resampler;
	
	producer = filefactory::GetBlockProducer( infilename);
	sink = filefactory::GetBlockSink( outfilename);

	if (producer && sink)
	{
		producer->GetStreamHeader( h);

		sof = ff.GetSampleFactory( h);
		if (sof)
		{
			my_resampler = sof->GetResampler();
			if (my_resampler)
			{
				my_resampler->Init( outfreq, infreq, lie);
				

				my_resampler->LinkTo( producer);
				sink->LinkTo( my_resampler);


				sink->Start();
				delete my_resampler;
			}
			delete sof;
		}
	}
	else puts(" could not open file");
	return 0;
}


