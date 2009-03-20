// TestPrologic.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "napl.h"
using namespace std;

bool make_stereo( const char *szleftname, const char *szrightname, const char *szoutputname)
{
	block_producer *pLeft = filefactory::GetBlockProducer( szleftname);
	if (!pLeft)
	{
		cout << "could not open sound file '" << szleftname << "'" << endl;
		return false;
	}

	block_producer *pRight = filefactory::GetBlockProducer( szrightname);
	if (!pRight)
	{
		cout << "could not open sound file '" << szrightname << "'" << endl;
		return false;
	}

	block_sink *pOutput = filefactory::GetBlockSink( szoutputname);
	if (!pOutput)
	{
		cout << "could not open output file" << endl;
		return false;
	}

	stream_header h;
	pLeft->GetStreamHeader( h);

	factory_factory ff;
	sample_object_factory *pFactory = ff.GetSampleFactory( h);

	binary_block_processor *pStereoMaker = pFactory->GetStereoMaker();
	
	pStereoMaker->LinkTo( pLeft, pRight);

	pOutput->LinkTo( pStereoMaker);

	pOutput->Start();

	delete pOutput;
	delete pStereoMaker;
	delete pLeft;
	delete pRight;

	return true;
}

bool make_back( const char *szinputname, const char *szoutputname)
{
	block_producer *pRight = filefactory::GetBlockProducer( szinputname);
	if (!pRight)
	{
		cout << "could not open sound file '" << szinputname << "'" << endl;
		return false;
	}

	block_producer *pLeft_pre = filefactory::GetBlockProducer( szinputname);
	if (!pLeft_pre)
	{
		cout << "could not open sound file '" << szinputname << "'" << endl;
		return false;
	}

	block_sink *pOutput = filefactory::GetBlockSink( szoutputname);
	if (!pOutput)
	{
		cout << "could not open output file" << endl;
		return false;
	}

	stream_header h;
	pLeft_pre->GetStreamHeader( h);

	factory_factory ff;
	sample_object_factory *pFactory = ff.GetSampleFactory( h);

	binary_block_processor *pStereoMaker = pFactory->GetStereoMaker();
	block_mutator *pNegator = pFactory->GetNegator();

	pNegator->LinkTo( pLeft_pre);
	pStereoMaker->LinkTo( pNegator, pRight);
	pOutput->LinkTo( pStereoMaker);

	pOutput->Start();

	delete pRight;
	delete pLeft_pre;
	delete pOutput;
	delete pStereoMaker;
	delete pNegator;
	return true;
}

bool make_crossfade( const char *szfirstname, const char *szlastname, const char *szoutputname)
{
	block_producer *pFrom = filefactory::GetBlockProducer( szfirstname);
	if (!pFrom)
	{
		cout << "could not open sound file '" << szfirstname << "'" << endl;
		return false;
	}

	block_producer *pTo = filefactory::GetBlockProducer( szlastname);
	if (!pTo)
	{
		cout << "could not open sound file '" << szlastname << "'" << endl;
		return false;
	}

	block_sink *pOutput = filefactory::GetBlockSink( szoutputname);
	if (!pOutput)
	{
		cout << "could not open output file" << endl;
		return false;
	}

	stream_header h;
	pFrom->GetStreamHeader( h);

	factory_factory ff;
	sample_object_factory *pFactory = ff.GetSampleFactory( h);
	block_mutator *pDelayedTo = pFactory->GetDelay( 5 /*ms*/);

	pDelayedTo->LinkTo( pTo);

	binary_block_processor *pCrossFader = pFactory->GetXFader();
	
	pCrossFader->LinkTo( pFrom, pDelayedTo);

	pOutput->LinkTo( pCrossFader);

	pOutput->Start();

	delete pOutput;
	delete pCrossFader;
	delete pFrom;
	delete pTo;

	return true;
}


int main(int argc, char* argv[])
{
	/*
	make_stereo("left.wav", "null.wav", "pro_left.wav");
	make_stereo("null.wav", "right.wav", "pro_right.wav");
	make_stereo("center.wav", "center.wav", "pro_center.wav");
	make_back( "back.wav", "pro_back.wav");
	paste_all();
	*/

	make_stereo( "mono.wav", "null.wav", "pro_left1.wav");
	make_back( "mono.wav", "pro_back1.wav");
	make_crossfade( "pro_left1.wav", "pro_back1.wav", "pro_mix_d.wav");

	return 0;
}

