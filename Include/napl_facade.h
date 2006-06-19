#pragma once

struct block_producer_wrapper: public boost::clipp::object
{
	block_producer *producer;

	explicit block_producer_wrapper( block_producer *producer_)
		:producer( producer_)
	{
		producer->GetStreamHeader( h);
	}

	block_producer_wrapper()
	{
	}

	/*
	operator block_producer *()
	{
		return producer;
	}*/

	// expose this class to the scripting engine
	static void init(boost::clipp::context * c);

	unsigned long get_samplerate(){ return get_header().samplerate;}; // frames per second
	short get_samplesize(){ return get_header().samplesize;}; // bits per sample
	unsigned short get_numchannels(){ return get_header().numchannels;}; // samples per frame
	unsigned long get_numframes(){ return get_header().numframes;};
	inline unsigned short get_frame_size(){ return get_header().frame_size();};

	~block_producer_wrapper()
	{
		//delete producer;
	}

private:
	stream_header &get_header()
	{
		return h;
	}

	stream_header h;

};

struct iterator_wrapper : public boost::clipp::object
{
private:
	stream_cut_iterator *m_iterator;
public:
	iterator_wrapper( stream_cut_iterator *_iterator)
		: m_iterator( _iterator)
	{
	}

	bool next()
	{
		return m_iterator->next();
	}

	bool at_end()
	{
		return m_iterator->at_end();
	}

	void reset()
	{
		m_iterator->reset();
	}

	block_producer_wrapper *current()
	{
		return new block_producer_wrapper( m_iterator);
	}

	// expose this class to the scripting engine
	static void init(boost::clipp::context * c);
};

struct sound_analysis_wrapper : public boost::clipp::object
{
	sound_analysis result;

	double get_max() { return result.max;}
	double get_min() { return result.min;}
	double get_avg() { return result.avg;}
	double get_norm() { return result.norm;}

	explicit sound_analysis_wrapper( const sound_analysis &result_)
		: result( result_)
	{
	}
	static void init( boost::clipp::context *c);

};

class file_reader: public boost::clipp::object
{
public:
	static void init(boost::clipp::context * c);


	// read one line of text
	std::string readln(void);
private:
	std::ifstream m_stream;
public:
	file_reader( std::string filename);
	// returns true when the file reader is at end-of-file or if it is in some error-state
	bool eof(void);
	void close(void);
	static file_reader * create_one(std::string filename);
};



class file_writer : public boost::clipp::object
{
private:
	std::ofstream m_stream;
public:
	static void init( boost::clipp::context * c);


	void write(std::string str);
	void writeln(std::string str);
	void close(void);
	file_writer(std::string filename);
	static file_writer * create_one(std::string filename);
};


class napl_facade: public boost::clipp::object
{

private:
	// test the block_producer pointer for NULL. Throw if is.
	static block_producer_wrapper *check_return(block_producer * producer, const std::string &message);
	static sample_object_factory * get_factory(block_producer_wrapper * source);
	static void check_compatibility( block_producer_wrapper *a, block_producer_wrapper *b);

public:
	// expose this class to the scripting engine
	static void init(boost::clipp::context * c);

	// create a block producer that reads from a file
	static block_producer_wrapper *read_file(std::string filename);

	// write the source to a file
	static void write_file(block_producer_wrapper *source, const std::string &filename);
	static block_producer_wrapper * amplify(block_producer_wrapper *source, double factor);
	// take a piece out of the input sample
	static block_producer_wrapper * cut(block_producer_wrapper * source, double start, double length);
	static block_producer_wrapper * cut_at_sample(block_producer_wrapper * source, unsigned long start, unsigned long length);

	static block_producer_wrapper * resample( block_producer_wrapper *source, unsigned short new_rate);
	// changes the speed (and pitch) of the sound without altering the samplerate
	static block_producer_wrapper * change_speed(block_producer_wrapper * source, double factor);
	// Joins two samples, placing the second_sample after the first
	static block_producer_wrapper * join(block_producer_wrapper * first_source, block_producer_wrapper * second_source);
	// create a stereo source out of two mono sources
	static block_producer_wrapper * make_stereo(block_producer_wrapper * left_channel, block_producer_wrapper * right_channel);
	static block_producer_wrapper * cross_fade(block_producer_wrapper * first_source, block_producer_wrapper * second_source);
	// negate
	static block_producer_wrapper * negate(block_producer_wrapper * source);
	// adds silence before and/or after a source
	static block_producer_wrapper * delay(block_producer_wrapper * source, double before, double after);
	// pans a stereo sound -1 = utter left 0 = no change 1 = utter right
	static block_producer_wrapper * pan(block_producer_wrapper * stereo_source, double pan_value);
	// find the min, max and average values of the samples in the source
	static sound_analysis_wrapper * analyze(block_producer_wrapper * source);
	// adds two samples by taking the average of each frame
	static block_producer_wrapper * add(block_producer_wrapper * source1, block_producer_wrapper * source2);
	// create a sound source
	static block_producer_wrapper * zero1(int channels, int samplesize, unsigned long samplerate, double seconds);
	// create a sound source
	static block_producer_wrapper * zero2( block_producer_wrapper *prototype);
	// creates a block producer that produces the sound as described in the 'prototype'-array. all values in the prototype-array should be within the range [-1, 1]
	static block_producer_wrapper * function(const std::vector<double> & prototype, unsigned short channel_count, int sample_size, unsigned long sample_rate, unsigned long frame_count);
	// only change the header of a sample, without changing the actual byte data. mostly used to change the samplerate
	static block_producer_wrapper * reinterpret(block_producer_wrapper * source, unsigned long samplerate, unsigned short channels = 0, short bits = 0);
	// extracts one channel from a multi-channel stream
	static block_producer_wrapper * extract_channel(block_producer_wrapper * source, short channel);
	// create an iterator on an input stream
	static iterator_wrapper * iterator(block_producer_wrapper * source, double window_size, double jump_size = 0.0);

	// modulate a sound source with a floating point sound stream...
	static block_producer_wrapper * amp_modulate(block_producer_wrapper * source, block_producer_wrapper * modulator);

	// convert the samples in the source stream to a different type.
	static block_producer_wrapper * convert(block_producer_wrapper * source, int samplesize);
};
