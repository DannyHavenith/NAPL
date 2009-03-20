#include "stdafx.h"
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "BladeMP3EncDLL.h"
#include ".\lame_wrapper.h"

struct lame_wrapper_impl
{
	HINSTANCE handle;
	BEINITSTREAM beInitStream;
	BEENCODECHUNK beEncodeChunk;
	BEDEINITSTREAM beDeinitStream;
	BECLOSESTREAM beCloseStream;
	BEWRITEVBRHEADER beWriteVBRHeader;
	BEWRITEINFOTAG beWriteInfoTag;

	lame_wrapper_impl()
		: handle(0)
	{
		// nop
	}

	~lame_wrapper_impl()
	{
		if (handle)
		{
			::FreeLibrary( handle);
		}
	}

	bool load( const char *lib_path = "lame_enc.dll")
	{
		handle = ::LoadLibrary( lib_path);

		if (!handle)
		{
			return false;
		}

		// Get Interface functions from the DLL
		beInitStream	= (BEINITSTREAM) ::GetProcAddress(handle, TEXT_BEINITSTREAM);
		beEncodeChunk	= (BEENCODECHUNK) ::GetProcAddress(handle, TEXT_BEENCODECHUNK);
		beDeinitStream	= (BEDEINITSTREAM) ::GetProcAddress(handle, TEXT_BEDEINITSTREAM);
		beCloseStream	= (BECLOSESTREAM) ::GetProcAddress(handle, TEXT_BECLOSESTREAM);
		beWriteVBRHeader= (BEWRITEVBRHEADER) ::GetProcAddress(handle,TEXT_BEWRITEVBRHEADER);
		beWriteInfoTag  = (BEWRITEINFOTAG) ::GetProcAddress(handle,TEXT_BEWRITEINFOTAG);

		if(!beInitStream || 
			!beEncodeChunk || 
			!beDeinitStream || 
			!beCloseStream || 
			!beWriteVBRHeader)
		{
			return false;
		}

		return true;
	};

};


lame_wrapper::lame_wrapper(void)
: pimpl(NULL)
{
	lame_wrapper_impl *new_pimpl = new lame_wrapper_impl();
	if (new_pimpl->load())
	{
		pimpl = new_pimpl;
	}
	else
	{
		delete new_pimpl;
	}
}

lame_wrapper::~lame_wrapper(void)
{
	delete pimpl;
}

lame_wrapper * lame_wrapper::get_instance(void)
{
	static std::auto_ptr<lame_wrapper> the_instance_ptr( new lame_wrapper);
	
	if (the_instance_ptr->is_ok())
	{
		return the_instance_ptr.get();
	}
	else
	{
		return 0;
	}

	return NULL;
}

bool lame_wrapper::is_ok(void)
{
	return pimpl != NULL;
}

lame_wrapper_impl *lame_wrapper::get_impl(void)
{
	if (!pimpl)
	{
		throw std::runtime_error( "mp3 encoder DLL could not be loaded");
	}

	return pimpl;
}

lame_stream * lame_wrapper::init_stream( unsigned long input_samplerate,
										unsigned short output_bitrate,
										unsigned short channels)
{
	BE_CONFIG	beConfig		={0,};
	HBE_STREAM stream_handle;

	memset(&beConfig,0,sizeof(beConfig));					// clear all fields

	// use the LAME config structure
	beConfig.dwConfig = BE_CONFIG_LAME;

	// this are the default settings for testcase.wav
	beConfig.format.LHV1.dwStructVersion	= 1;
	beConfig.format.LHV1.dwStructSize		= sizeof(beConfig);		
	beConfig.format.LHV1.dwSampleRate		= input_samplerate;				// INPUT FREQUENCY
	beConfig.format.LHV1.dwReSampleRate		= 0;					// DON"T RESAMPLE
	if (channels == 2)
	{
		beConfig.format.LHV1.nMode = BE_MP3_MODE_JSTEREO;	// OUTPUT IN STREO

	} // if (channels == 2)
	else if (channels ==1)
	{
		beConfig.format.LHV1.nMode = BE_MP3_MODE_MONO;
	}
	else
	{
		throw std::runtime_error( "mp3 output should be 1- or 2-channel");
	}
	beConfig.format.LHV1.dwBitrate			= output_bitrate;					// MINIMUM BIT RATE
	beConfig.format.LHV1.nPreset			= LQP_R3MIX;		// QUALITY PRESET SETTING
	beConfig.format.LHV1.dwMpegVersion		= MPEG1;				// MPEG VERSION (I or II)
	beConfig.format.LHV1.dwPsyModel			= 0;					// USE DEFAULT PSYCHOACOUSTIC MODEL 
	beConfig.format.LHV1.dwEmphasis			= 0;					// NO EMPHASIS TURNED ON
	beConfig.format.LHV1.bOriginal			= TRUE;					// SET ORIGINAL FLAG
	beConfig.format.LHV1.bWriteVBRHeader	= TRUE;					// Write INFO tag
	beConfig.format.LHV1.bNoRes				= TRUE;					// No Bit resorvoir

	// Init the MP3 Stream
	err = beInitStream(&beConfig, &dwSamples, &dwMP3Buffer, &hbeStream);

	return NULL;
}
