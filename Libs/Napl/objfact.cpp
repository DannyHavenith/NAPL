//
// 
//
#include "stdafx.h"
#include "processor_visitor.h"
#include "sample_analyzer.h"
#include "objfact.h"
#include "convendi.h"
#include "samplety.h"
#include "truncate.h"
#include "uniform_block_converter.h"
#include "resample.h"
#include "xfade.h"
#include "stereomaker.h"
#include "negator.h"
#include "delay.h"
#include "pan_impl.h"
#include "constant_producer.h"
#include "analyzer.h"
#include "amplifier.h"
#include "function_impl.h"
#include "extract_channel.h"
#include "type_converter.h"
#include "amp_modulator.h"

#include "sample_factory_instantiator.h"


int test = truncate<unsigned char>( sampletype_m16(300));

template< typename sampletype>
block_mutator *create_pan( short pan_value, const StereoSample<sampletype> &)
{
	return new stereo_pan< sampletype>( pan_value);
};

block_mutator *create_pan( short pan_value, const sampletype_m24 &)
{
	return 0;
};

block_mutator *create_pan( short pan_value, const sampletype_m16 &)
{
	return 0;
};

block_mutator *create_pan( short pan_value, const sampletype_m8 &)
{
	return 0;
};

block_mutator *create_pan( short pan_value, const sampletype_md &)
{
	return 0;
};



template< typename sampletype>
sample_analyzer *t_sample_object_factory<sampletype>::GetAnalyzer()
{
	return new typed_analyzer<sampletype>();
}

template< typename sampletype>
block_mutator *t_sample_object_factory<sampletype>::GetChannelExtractor( short channel)
{
	return new channel_extract_mutator<sampletype>( channel);
}

template< typename sampletype>
block_mutator *t_sample_object_factory<sampletype>::GetPan( short pan_value)
{
	sampletype p;
	return create_pan( pan_value, p);
}

template< typename sampletype>
block_producer *t_sample_object_factory<sampletype>::GetConstant( stream_header h)
{
	return new constant_producer<sampletype>( h);
}

template< typename sampletype>
block_mutator *t_sample_object_factory<sampletype>::GetResampler( 
	unsigned long outfreq, 
	unsigned long infreq, 
	bool lie)
{
	return new resampler< sampletype>( outfreq, infreq, lie);
}

template< typename sampletype>
block_mutator *t_sample_object_factory<sampletype>::GetNegator()
{
	return new negator< sampletype>();
}

template< typename input_sample_type>
binary_block_processor * t_sample_object_factory<input_sample_type>::GetStereoMaker()
{
	return new stereo_maker< input_sample_type>();
}

template< typename sampletype>
block_mutator *t_sample_object_factory<sampletype>::GetEndianConverter()
{
	return static_cast< block_mutator *> (new endian_converter< sampletype>());
}

template< typename sampletype>
block_mutator *t_sample_object_factory<sampletype>::GetDelay( double delay_seconds, double post_seconds)
{
	return new delay< sampletype>( delay_seconds, post_seconds);
}

template< typename sampletype>
binary_block_processor *t_sample_object_factory<sampletype>::GetXFader()
{
	return static_cast< binary_block_processor *> ( new xfader< sampletype>());
}

template< typename sampletype>
binary_block_processor *t_sample_object_factory<sampletype>::GetAdder()
{
	return new binary_block_mutator< mut_add<sampletype> >();
}

template< typename sampletype>
block_mutator *t_sample_object_factory<sampletype>::GetAmplifier( double factor)
{
	return new amplifier<sampletype>( factor);
}

template< typename sampletype>
block_producer *t_sample_object_factory<sampletype>::GetFunction( const std::vector<double> &prototype, const stream_header &h)
{
	return new function<sampletype>( prototype, h);
}

template< typename sampletype>
binary_block_processor * t_sample_object_factory<sampletype>::GetAmpModulator()
{
	return new asymmetric_binary_block_mutator< amp_modulator_mutator< double, sampletype> >();
}


sample_object_factory *factory_factory::GetSampleFactory( const stream_header &h)
{
	return sample_factory_instantiator<t_sample_object_factory, sample_object_factory>::instantiate( h);
}


block_producer *GlobalGetEndianConverter( const stream_header &h, block_producer *pP)
{
	block_mutator *endianConverter;

	sample_object_factory *sof = factory_factory::GetSampleFactory( h);
	endianConverter = sof->GetEndianConverter( );
	endianConverter -> LinkTo( pP);

	delete sof;

	return endianConverter;
}