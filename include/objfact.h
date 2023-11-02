////////////////////////////////////////////////////////////////////////////////
//
// objfact.h - definition of the object factories needed for several sample-
// and file types.
//

#ifndef _OBJECT_FACTORY_H_
#define _OBJECT_FACTORY_H_

#include "samplebl.h"
class from_to_factory
{
public:
	virtual block_mutator *GetConverter() = 0;
};

class general_resampler;

class sample_object_factory
{
public:
	virtual binary_block_processor *GetXFader() = 0;
	virtual binary_block_processor *GetStereoMaker() = 0;
	virtual general_resampler *GetResampler( unsigned long outfreq, unsigned long infreq, bool lie) = 0;
	virtual block_mutator *GetNegator() = 0;
	virtual block_mutator *GetEndianConverter() = 0;
	virtual binary_block_processor *GetAdder() = 0;
	virtual block_mutator *GetDelay( double delay_seconds, double post_seconds = 0 ) = 0;
	virtual block_mutator *GetPan( short pan_value) = 0;
	virtual block_producer *GetConstant( stream_header h) = 0;
	virtual sample_analyzer *GetAnalyzer() = 0;
	virtual block_mutator *GetAmplifier( double factor) = 0;
	virtual block_producer *GetFunction( const std::vector<double> &prototype, const stream_header &h) = 0;
	virtual block_mutator *GetChannelExtractor( short channel) = 0;
	virtual binary_block_processor *GetAmpModulator() = 0;
	virtual from_to_factory *GetFromToFactory( const stream_header &to_header) = 0;
	//virtual block_mutator *GetFFT( int power) = 0;

	virtual ~sample_object_factory() = default;
};

struct t_sample_object_factory
{
	template<typename sampletype>
	class apply: public sample_object_factory
	{
	public:
		virtual general_resampler *GetResampler(unsigned long outfreq,
				unsigned long infreq, bool lie);
		virtual binary_block_processor *GetStereoMaker();
		virtual block_mutator *GetEndianConverter();
		virtual binary_block_processor *GetXFader();
		virtual block_mutator *GetNegator();
		virtual binary_block_processor *GetAdder();
		virtual block_mutator *GetDelay(double delay_seconds,
				double post_seconds = 0);
		virtual block_mutator *GetPan(short pan_value);
		virtual block_producer *GetConstant(stream_header h);
		virtual sample_analyzer *GetAnalyzer();
		virtual block_mutator *GetAmplifier(double factor);
		virtual block_producer *GetFunction(
				const std::vector<double> &prototype, const stream_header &h);
		virtual block_mutator *GetChannelExtractor(short channel);
		virtual binary_block_processor *GetAmpModulator();
		virtual from_to_factory *GetFromToFactory(
				const stream_header &from_header);
	private:

		//virtual block_mutator *GetFFT( int power);
	};
};

class factory_factory
{
public:
	static sample_object_factory *GetSampleFactory( const stream_header &h);
};
#endif
