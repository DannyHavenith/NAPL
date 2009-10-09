#if !defined( MPG_FILE_HPP)
#define MPG_FILE_HPP
#include "architec.h"
#include "samplebl.h"

class mp3_block_producer : public creative_block_producer
{
public:
	virtual void GetStreamHeader( stream_header &h);

    mp3_block_producer( const char *filename);

	virtual ~mp3_block_producer();


protected:
	virtual void Seek( sampleno start);
	virtual sampleno FillBlock( sample_block &b, sampleno count);
private:
    struct implementation;

    implementation *impl;

};

#endif //MPG_FILE_HPP