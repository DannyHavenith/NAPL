/*
        mpg123_to_wav.c

        copyright 2007 by the mpg123 project - free software under the terms of the LGPL 2.1
        see COPYING and AUTHORS files in distribution or http://mpg123.org
        initially written by Nicholas Humfrey
*/
#include <cstdio>
#include <mpg123.h>
#include <stdexcept>
#include "mpg_file.hpp"

using namespace std;

//=========================================
/// make sure mpg123 is initialized and de-initialized.
struct mpg123_initializer
{
    mpg123_initializer()
    {
         mpg123_init();
    }
    ~mpg123_initializer()
    {
        mpg123_exit();
    }
} initializer;
struct mp3_block_producer::implementation
{
    mpg123_handle *mh;
    stream_header header;

    implementation( const char *filename)
        :mh(0)
    {
        long rate;
        int channels;
        int encoding;
        int err = MPG123_OK;

        mh = mpg123_new(NULL, &err);
        throw_if_error( err);

        throw_if_error(
            mpg123_open( mh, filename)
            );

        throw_if_error(
            mpg123_getformat( mh, &rate, &channels, &encoding)
            );

        mpg123_format_none(mh);
        mpg123_format(mh, rate, channels, encoding);

        // scan the whole file to get an accurate length estimate.
        mpg123_scan(mh);

	    header.samplerate = rate; // frames per second
	    header.samplesize = 16; // bits per sample, negative values have special meanings.
	    header.numchannels = channels; // samples per frame
	    header.numframes = mpg123_length( mh);
	    header.architecture = LOCAL_ARCHITECTURE; ///< endian-nes and interleaving of samples
    }

    size_t read( unsigned char*buffer, size_t buffer_size)
    {
        size_t done;
        mpg123_read( mh, buffer, buffer_size, &done);
        return done;
    }

    void seek( size_t sample_offset)
    {
        mpg123_seek( mh, sample_offset, SEEK_SET);
    }

    void throw_if_error( int code)
    {
        if (code != MPG123_OK)
        {
            std::string error = mh?mpg123_strerror(mh):mpg123_plain_strerror( code);
            // since this function may be called from the c'tor, which means the d'tor wil not be
            // called, we need to cleanup.
            cleanup();
            throw std::runtime_error( error);
        }
    }

    ~implementation()
    {
        cleanup();
    }

    void cleanup()
    {
        mpg123_close(mh);
        mpg123_delete(mh);
        mh = 0;
    }
};


void mp3_block_producer::GetStreamHeader( stream_header &h)
{
    h = impl->header;
}

mp3_block_producer::mp3_block_producer( const char *filename)
{
    impl = new implementation( filename);
}

mp3_block_producer::~mp3_block_producer()
{
    delete impl;
}

void mp3_block_producer::Seek( sampleno start)
{
    impl->seek( start);
}

sampleno mp3_block_producer::FillBlock( sample_block &b, sampleno count)
{
    short framesize = impl->header.frame_size();

    if (count * framesize > b.buffer_size())
    {
        count = sampleno( b.buffer_size() / framesize);
    }

    b.m_end = b.m_start + count * framesize;

    sampleno nRead = static_cast<sampleno>( 
        impl->read( b.buffer_begin(), count * framesize) / framesize
        );

    return nRead;
}


//==========================================
