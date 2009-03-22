#include "stdafx.h"
#include "sample_analyzer.h"
#include "objfact.h"
#include "convendi.h"
#include "samplety.h"

#include "uniform_block_converter.h"
#include "type_converter.h"
#include "sample_factory_instantiator.h"

template< typename from_type, typename to_type>
struct t_double_dispatch_factory : public from_to_factory
{
	block_mutator *GetConverter()
	{
		typedef typename sampletraits<to_type>::channel_type channel_type;
		typedef typename sampletraits<from_type>::template foreach_channel_type< channel_type>::type corrected_type;

		return new type_conversion_mutator<from_type, corrected_type>();
	}
};

template< typename from_type>
struct first_factory
{
	struct second_factory
	{
		template< typename to_type>
		struct apply: public t_double_dispatch_factory< from_type, to_type>
		{
		};
	};
};

template< typename sampletype>
from_to_factory *t_sample_object_factory::apply<sampletype>::GetFromToFactory( const stream_header &h)
{
	typedef sample_factory_instantiator<
		typename first_factory< sampletype>::second_factory,
		from_to_factory>
		instantiator;

	return instantiator::instantiate( h);
}

template from_to_factory *t_sample_object_factory::apply<sampletype_s16>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_m16>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_q16>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_s24>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_m24>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_q24>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_s8>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_m8>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_q8>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_sd>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_md>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_qd>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_mf>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_sf>::GetFromToFactory( const stream_header &h);
template from_to_factory *t_sample_object_factory::apply<sampletype_qf>::GetFromToFactory( const stream_header &h);
