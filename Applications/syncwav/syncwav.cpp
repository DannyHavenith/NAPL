// syncwav.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma warning( disable:4786)
//
// quick and dirty argument parsing
//
using std::cout;
using std::endl;

struct CArguments
{
	CArguments()
		:m_bVerbose( false)
	{
	}

	bool parse( int argc, char *argv[])
	{
		int nCurrentArg = 1;
		if (argc < 3)
		{
			cout << "usage: syncwav [-v] <FileListFileName> <InputWav>" << endl;
			return false;
		}

		//
		// scan command-line flags
		//
		while (nCurrentArg < argc-2 && argv[nCurrentArg][0] == '-')
		{
			if (argv[nCurrentArg][1] == 'v')
			{
				m_bVerbose = true;
			}
			++nCurrentArg;
		}

		m_strAviFileListFile = argv[nCurrentArg++];
		m_strInputSoundFile = argv[nCurrentArg++];

		return true;
	};

	bool m_bVerbose;
	std::string m_strAviFileListFile;
	std::string m_strInputSoundFile;
};

typedef std::vector< std::string> CFileNameList;

struct CSlice 
{
	CSlice()
		:m_nMilliSeconds(0)
	{
	}

	CSlice( long nMiliSeconds, std::string strFileNameBase)
		: m_nMilliSeconds( nMiliSeconds),
		m_strFileNameBase( strFileNameBase)
	{
		// nop
	};

	unsigned long m_nMilliSeconds;
	std::string m_strFileNameBase;
};

//
// this type stores the cutting information we need in the sample
//
typedef std::vector< CSlice> CSlices;

bool GetFileNames( CArguments &args, CFileNameList &files)
{
	// open file
	std::ifstream strmFileListFile( args.m_strAviFileListFile.c_str());

	// check file
	if (!strmFileListFile) 
	{
		cout << "could not open file: " << args.m_strAviFileListFile << endl;
		return false;
	}

	// 
	// get filenames
	std::string strFileName;
	while ( std::getline( strmFileListFile, strFileName))
	{
		if (args.m_bVerbose) cout << "Adding file " << strFileName << endl;
		files.push_back( strFileName);
	}

	return true;
}

bool GetSlices( const CFileNameList &listFileNames, CSlices &slices, const CArguments &args)
{
	CFileNameList::const_iterator i = listFileNames.begin();

	for (;i != listFileNames.end(); ++i)
	{
		IAVIFile * pAviFile;
		IAVIStream *pAviStream;
		AVISTREAMINFO streaminfo;

		if (::AVIFileOpen( &pAviFile, i->c_str(), OF_READ, 0))
		{
			cout << "could not open '" << i->c_str() << "' for reading" << endl;
			continue;
		}
		
		if (::AVIFileGetStream( pAviFile, &pAviStream, streamtypeVIDEO, 0))
		{
			::AVIFileRelease( pAviFile);
			cout << "file '" << i->c_str() << "' contains no video stream" << endl;
			continue;
		}
		
		::AVIStreamInfo( pAviStream, &streaminfo, sizeof( streaminfo));
		
		if (args.m_bVerbose)
		{
			cout << "Opened file '" << i->c_str() << "' length=" << streaminfo.dwLength << " scale =" << streaminfo.dwScale
				<< " rate= " << streaminfo.dwRate << endl;
			cout << "calculated size: " << unsigned long (streaminfo.dwLength * (float(streaminfo.dwScale) / float(streaminfo.dwRate)  * 1000.0)) << "ms" << endl;
		}

		slices.push_back( 
			CSlice( streaminfo.dwLength * (float(streaminfo.dwScale) / float(streaminfo.dwRate)  * 1000.0),
					i->substr( 0, i->find_last_of( '.'))));
		pAviStream->Release();
		pAviFile->Release();
	}

	return true;
}

bool CreatePartialFile( std::string strFileNameBase, unsigned long nBegin, unsigned long nEnd, block_producer *pProducer)
{
	cut_mutator cutter;
	block_sink *pSink = filefactory::GetBlockSink( (strFileNameBase + ".wav").c_str());
	if (!pSink) return false;

	cutter.LinkTo( pProducer);
	pSink->LinkTo( &cutter);
	cutter.SetCut( nBegin, nEnd - nBegin);
	pSink->Start();

	delete pSink;

	return true;
}

#define PRINTMEMBER( object, member) cout << #member ": "<< object.member << endl;
void PrintHeader( const stream_header &h)
{
	PRINTMEMBER( h,numchannels);
	PRINTMEMBER( h,numframes);
	PRINTMEMBER( h,samplerate);
	PRINTMEMBER( h,samplesize);
}

bool CreatePartials( const CSlices &slices, const CArguments args)
{
	unsigned long nCurrentBegin = 0;
	unsigned long nCurrentEnd = 0;

	block_producer *pProducer = filefactory::GetBlockProducer( args.m_strInputSoundFile.c_str());
	if (!pProducer)
	{
		cout << "could not open sound file '" << args.m_strInputSoundFile << "'" << endl;
		return false;
	}


	//
	// get header info for the stream...
	//
	stream_header h;
	pProducer->GetStreamHeader( h);

	if (args.m_bVerbose)
	{
		cout << "Loaded file: " << args.m_strInputSoundFile << endl;
		PrintHeader( h);
		cout << "calculated size: " << h.numframes / h.samplerate << "s (" << (h.numframes / h.samplerate) / 60 << " min)" << endl;
	}

	CSlices::const_iterator i;
	for (i = slices.begin(); nCurrentBegin < h.numframes && i != slices.end(); ++i)
	{
		nCurrentEnd = nCurrentBegin + (__int64(h.samplerate)  * __int64(i->m_nMilliSeconds)) / 1000;
		CreatePartialFile( i->m_strFileNameBase, nCurrentBegin, nCurrentEnd, pProducer);
		nCurrentBegin = nCurrentEnd;
	}
	
	if (nCurrentEnd < h.numframes)
	{
		cout << h.numframes - nCurrentEnd << " unassigned samples left" << endl;
	}
	
	delete pProducer;

	return true;
}

int main(int argc, char* argv[])
{
	CArguments args;
	if (!args.parse( argc, argv)) return -1;

	CFileNameList listFileNames;
	if (!GetFileNames( args, listFileNames)) return -2;

	CSlices slices;
	if (!GetSlices( listFileNames, slices, args)) return -3;

	if (!CreatePartials( slices, args)) return -4;
	
	return 0;
}

