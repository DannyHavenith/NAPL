#include "StdAfx.h"
#include "napl_facade.h"
#include "..\..\include\napl_facade.h"


using std::string;
using namespace boost::clipp;



// create a block producer that reads from a file
block_producer_wrapper *napl_facade::read_file(std::string filename)
{
	return check_return(
		filefactory::GetBlockProducer( filename.c_str()),
		string( "could not open file '") + filename + "' for reading"
		);
}

// test the block_producer pointer for NULL. Throw if is.
block_producer_wrapper *napl_facade::check_return(block_producer * producer, const string &message)
{
	if (!producer) throw napl_error( message);
	return new block_producer_wrapper( producer);
}

// write the source to a file
void napl_facade::write_file(block_producer_wrapper *source, const std::string &filename)
{
	block_sink *file = filefactory::GetBlockSink( filename.c_str());
	if (!file) throw napl_error( "could not open file '" + filename + "' for writing");

	text_based_progress_bar bar( "writing to " + filename, std::cout, 79);
	progress_monitor mon( &bar);

	mon.LinkTo( source->producer);
	file->LinkTo( &mon);
	file->Start();
	delete file;
}
void file_reader::init( boost::clipp::context * c)
{
	class_<file_reader> cls( "FileReader", c);
	cls.constructor( arguments< std::string>());
	cls.function( "readln", readln);
	cls.function( "eof", eof);
	cls.function( "close", close);
}

void file_writer::init( boost::clipp::context * c)
{
	class_<file_writer> cls( "FileWriter", c);
	cls.constructor( arguments< std::string>());
	cls.function( "writeln", writeln);
	cls.function( "write", write);
	cls.function( "close", close);
}

// expose this class to the scripting engine
void napl_facade::init(boost::clipp::context * c)
{
	class_<napl_facade> cls( "Napl", c);
	cls.static_function( "read_file", read_file);
	cls.static_function( "write_file", write_file);
	cls.static_function( "analyze", analyze);
	cls.static_function( "amplify", amplify);
	cls.static_function( "truncate", truncate);
	cls.static_function( "cut", cut)
		.signature( "sound", "pos",arg("size")=0);
	cls.static_function( "cut_at_sample", cut_at_sample)
		.signature( "sound", "pos", arg("size") = 0);
	cls.static_function( "resample", resample);
	cls.static_function( "change_speed", change_speed);
	cls.static_function( "join", join);
	cls.static_function( "make_stereo", make_stereo);
	cls.static_function( "negate", negate);
	cls.static_function( "delay", delay);
	cls.static_function( "pan", pan);
	cls.static_function( "add", add);
	cls.static_function( "zero", zero1);
	cls.static_function( "zero", zero2);
	cls.static_function( "func", function);
	cls.static_function( "reinterpret", reinterpret)
		.signature( "sound", "rate", arg("channels") = 0, arg( "bits") = 0);
	cls.static_function( "extract_channel", extract_channel);
	cls.static_function( "iterator", iterator)
		.signature( "source", "window_size", arg("jump_size") = 0.0);
	cls.static_function( "amp_modulate", amp_modulate);

	//
	// also add the file reader and -writer to the environment
	//
	file_reader::init( c);
	file_writer::init( c);
}

void block_producer_wrapper::init(boost::clipp::context * c)
{
	class_<block_producer_wrapper> cls( "block_producer", c);
	cls.read( "sample_rate", get_samplerate);
	cls.read( "sample_size", get_samplesize);
	cls.read( "channel_count", get_numchannels);
	cls.read( "frame_count", get_numframes);
	cls.read( "frame_size", get_frame_size);

}

void iterator_wrapper::init( boost::clipp::context *c)
{
	class_<iterator_wrapper> cls("iterator", c);
	cls.read( "at_end", at_end);
	cls.function( "next", next);
	cls.function( "current", current);
	cls.function( "reset", reset);
}

void sound_analysis_wrapper::init( boost::clipp::context *c)
{
	class_<sound_analysis_wrapper> cls( "sound_analysis", c);
	cls.read( "max", get_max);
	cls.read( "min", get_min);
	cls.read( "avg", get_avg);
	cls.read( "norm", get_norm);
}

// find the min, max and average values of the samples in the source
sound_analysis_wrapper * napl_facade::analyze(block_producer_wrapper * source)
{

	sample_analyzer *an = get_factory( source)->GetAnalyzer();
	text_based_progress_bar bar( "analyzing samples", std::cout, 79);
	progress_monitor mon( &bar);

	mon.LinkTo( source->producer);
	an->LinkTo( &mon);

	nil_sink sink;
	sink.LinkTo( an);
	sink.Start();

	sound_analysis result = an->GetResult();

	delete an;

	return new sound_analysis_wrapper( result);
}

sample_object_factory * napl_facade::get_factory(block_producer_wrapper * source)
{
	stream_header h;
	source->producer->GetStreamHeader( h);
	return factory_factory::GetSampleFactory( h);
}

block_producer_wrapper * napl_facade::amplify(block_producer_wrapper *source, double factor)
{
	block_mutator *amp = get_factory( source)->GetAmplifier( factor);
	amp->LinkTo( source->producer);

	return check_return( amp,
		"could not create an amplifier");
}

// low-level truncation of samples: this will just remove the least significant bytes from each sample
block_producer_wrapper * napl_facade::truncate(block_producer_wrapper * source, int byte_size)
{
	stream_header h;
	source->producer->GetStreamHeader( h);
	if (h.frame_size() < byte_size) 
	{
		throw napl_error( "cannot truncate if source framesize is smaller than the destination size");
	}

	block_mutator *trunc = new truncator( byte_size);
	trunc->LinkTo( source->producer);
	return new block_producer_wrapper( trunc);
}

// take a piece out of the input sample
block_producer_wrapper * napl_facade::cut(block_producer_wrapper * source, double start, double length)
{
	stream_header h;
	source->producer->GetStreamHeader( h);
	start *= h.samplerate;
	length *= h.samplerate;

	cut_mutator *cutter = new cut_mutator();
	cutter->LinkTo( source->producer);
	cutter->SetCut( sampleno( start), sampleno( length));

	return check_return( cutter, "could not cut sample");
}

// take a piece out of the input sample
block_producer_wrapper * napl_facade::cut_at_sample(block_producer_wrapper * source, unsigned long start, unsigned long length)
{

	cut_mutator *cutter = new cut_mutator();
	cutter->LinkTo( source->producer);
	cutter->SetCut( sampleno( start), sampleno( length));

	return check_return( cutter, "could not cut sample");
}

block_producer_wrapper * napl_facade::resample( block_producer_wrapper *source, unsigned short new_rate)
{
	stream_header h;
	source->producer->GetStreamHeader( h);

	block_mutator *resampler = get_factory( source)->GetResampler( new_rate, h.samplerate, false);
	resampler->LinkTo( source->producer);

	return check_return( resampler, "could not resample");
}

// changes the speed (and pitch) of the sound without altering the samplerate
block_producer_wrapper * napl_facade::change_speed(block_producer_wrapper * source, double factor)
{
	stream_header h;
	source->producer->GetStreamHeader( h);

	unsigned long new_rate = h.samplerate;
	unsigned long old_rate = h.samplerate;

	if (factor > 0.0)
	{
		if (factor > double( new_rate))
		{
			throw napl_error( string( "speed change too high (sound reduced to single sample)"));
		}
		new_rate /= factor;

	}
	else
	{
		if (factor * old_rate < 1.0)
		{
			throw napl_error( string( "speed too low") );
		}
		old_rate *= factor;
	}



	block_mutator *resampler = get_factory( source)->GetResampler( new_rate, old_rate, true);
	resampler->LinkTo( source->producer);

	return check_return( resampler, "could not resample");
}

void napl_facade::check_compatibility( block_producer_wrapper *a, block_producer_wrapper *b)
{
	stream_header ha, hb;
	a->producer->GetStreamHeader( ha);
	b->producer->GetStreamHeader( hb);

	if (
		ha.architecture != hb.architecture ||
		ha.numchannels != hb.numchannels ||
		ha.samplerate != hb.samplerate ||
		ha.samplesize != hb.samplesize )
	{
		throw napl_error( "streams were incompatible");
	}
}

// Joins two samples, placing the second_sample after the first
block_producer_wrapper * napl_facade::join(block_producer_wrapper * first_source, block_producer_wrapper * second_source)
{
	check_compatibility( first_source, second_source);
	paste_mutator *joiner = new paste_mutator();
	joiner->LinkTo( first_source->producer, 0);
	joiner->LinkTo( second_source->producer, 1);

	return new block_producer_wrapper( joiner);
}

// create a stereo source out of two mono sources
block_producer_wrapper * napl_facade::make_stereo(block_producer_wrapper * left_channel, block_producer_wrapper * right_channel)
{
	check_compatibility( left_channel, right_channel);
	binary_block_processor *stereo_maker = get_factory( left_channel)->GetStereoMaker();


	stereo_maker->LinkTo( left_channel->producer, right_channel->producer);
	block_producer_wrapper *result = check_return( stereo_maker, "could not create a stereo sound");

	return result;
}

//  fade from one source to the next
block_producer_wrapper * napl_facade::cross_fade(block_producer_wrapper * first_source, 
												  block_producer_wrapper * second_source)
{
	check_compatibility( first_source, second_source);
	binary_block_processor *crossfader= get_factory( first_source)->GetXFader();



	crossfader->LinkTo( first_source->producer, second_source->producer);
	block_producer_wrapper *result = check_return( crossfader, "could not cross-fade");

	return result;
}

// negate
block_producer_wrapper * napl_facade::negate(block_producer_wrapper * source)
{
	block_mutator *neg = get_factory( source)->GetNegator();
	neg->LinkTo( source->producer);
	return check_return( neg, "could not negate");
}


// adds silence before and/or after a source
block_producer_wrapper * napl_facade::delay(block_producer_wrapper * source, double before, double after)
{
	block_mutator *delay = get_factory( source)->GetDelay( before, after);
	delay->LinkTo( source->producer);
	return check_return( delay, "could not create delay");
}


// pans a stereo sound -1 = utter left 0 = no change 1 = utter right
block_producer_wrapper * napl_facade::pan(block_producer_wrapper * stereo_source, double pan_value)
{
	
	// NAPL pan expects an integer pan value between 0 and 65535
	double recalc = (pan_value * 32768 + 32768);
	if (recalc > 65535.0) recalc = 65535.0;
	if (recalc < 0.0) recalc = 0.0;
	unsigned short fixed_pan = static_cast<unsigned short>( recalc);
	
	block_mutator *panner = get_factory( stereo_source)->GetPan( fixed_pan);
	panner->LinkTo( stereo_source->producer);
	block_producer_wrapper *result = check_return( panner,
			"could not create stereo pan from input (mono sample?)");

	return result;
}

// adds two samples by taking the average of each frame
block_producer_wrapper * napl_facade::add(block_producer_wrapper * source1, block_producer_wrapper * source2)
{
	check_compatibility( source1, source2);
	binary_block_processor *adder = get_factory( source1)->GetAdder();
	adder->LinkTo( source1->producer, source2->producer);
	block_producer_wrapper *result = check_return( adder, "could not add");
	return result;
}

// create a sound source
block_producer_wrapper * napl_facade::zero1(int channels, 
											int samplesize, 
											unsigned long samplerate, 
											double seconds)
{
	stream_header h;
	h.architecture = LOCAL_ARCHITECTURE;
	h.numchannels = channels;
	h.numframes = unsigned long ( seconds * samplerate);
	h.samplerate = samplerate;
	h.samplesize = samplesize;

	return check_return(  factory_factory::GetSampleFactory( h)->GetConstant( h),
						"could not create a zero sound with the given specifications");

}

// create a sound source
block_producer_wrapper * napl_facade::zero2( block_producer_wrapper *prototype)
{
	stream_header h;
	prototype->producer->GetStreamHeader( h);
	return check_return(  factory_factory::GetSampleFactory( h)->GetConstant( h),
						"could not create a zero sound with the given specifications");

}

// creates a block producer that produces the sound as described in the 'prototype'-array. all values in the prototype-array should be within the range [-1, 1]
block_producer_wrapper * napl_facade::function(const std::vector<double> & prototype, 
											   unsigned short channel_count, 
											   int samplesize,
											   unsigned long sample_rate, 
											   unsigned long frame_count)
{
	stream_header h;

	h.architecture = LOCAL_ARCHITECTURE;
	h.numchannels = channel_count;
	h.numframes = frame_count;
	h.samplerate = sample_rate;
	h.samplesize = samplesize;


	return check_return( factory_factory::GetSampleFactory( h)->GetFunction( prototype, h),
		"unable to create function source");
}

// only change the header of a sample, without changing the actual byte data. mostly used to change the samplerate
block_producer_wrapper * napl_facade::reinterpret(block_producer_wrapper * source, unsigned long samplerate, unsigned short channels, short bits)
{

	stream_header h;
	source->producer->GetStreamHeader( h);
	if (samplerate)
	{
		h.samplerate = samplerate;
	}

	if (channels)
	{
		h.numchannels = channels;
	}

	if (bits)
	{
		h.samplesize = bits;
	}


	block_mutator *preint = new reinterpret_mutator( h);
	preint->LinkTo( source->producer);

	return new block_producer_wrapper( preint);
}

// extracts one channel from a multi-channel stream
block_producer_wrapper * napl_facade::extract_channel(block_producer_wrapper * source, short channel)
{

	block_mutator *extractor = get_factory( source)->GetChannelExtractor( channel);
	if (!extractor)
	{
		throw napl_error( "could not create channel extractor");
	}
	extractor->LinkTo( source->producer);
	return new block_producer_wrapper( extractor);
}

// create an iterator on an input stream
iterator_wrapper * napl_facade::iterator(block_producer_wrapper * source, double window_size, double jump_size)
{
	stream_header h;
	source->producer->GetStreamHeader( h);

	//
	// comparing a double, should be OK, since the value 0.0 should be assigned to the parameter and
	// not calculated
	if ( jump_size == 0.0)
	{
		jump_size = window_size;
	}

	stream_cut_iterator *it = new stream_cut_iterator( window_size * h.samplerate, jump_size * h.samplerate);
	it->LinkTo( source->producer);


	return new iterator_wrapper( it);
}

// modulator
block_producer_wrapper * napl_facade::amp_modulate(block_producer_wrapper * source, block_producer_wrapper * modulator)
{
	stream_header h;
	modulator->producer->GetStreamHeader( h);
	if ( h.samplesize != -2)
	{
		throw napl_error( "modulator sound source must be of floating point type");
	}

	binary_block_processor *mod = get_factory( source)->GetAmpModulator();

	mod->LinkTo( modulator->producer, source->producer);

	return new block_producer_wrapper( mod);

}


// read one line of text
std::string file_reader::readln(void)
{
	std::string buffer;
	std::getline( m_stream, buffer);
	return buffer;
}

file_reader::file_reader( std::string filename)
	:m_stream( filename.c_str())
{
}

// returns true when the file reader is at end-of-file or if it is in some error-state
bool file_reader::eof(void)
{
	return !m_stream;
}

void file_reader::close(void)
{
	m_stream.close();
}

void file_writer::write(std::string str)
{
	m_stream << str;
}

void file_writer::writeln(std::string str)
{
	write( str);
	m_stream << std::endl;
}

void file_writer::close(void)
{
	m_stream.close();
}

file_writer::file_writer(std::string filename)
:m_stream( filename.c_str())
{
}

file_writer * file_writer::create_one(std::string filename)
{
	return new file_writer( filename);
}

file_reader * file_reader::create_one(std::string filename)
{
	return new file_reader( filename);
}
