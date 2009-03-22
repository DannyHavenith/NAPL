////////////////////////////////////////////////////////////////////////////////
//
// samplety.h - definition of the types of samples used
//
// This file currently describes 4 sampletypes:
//
// 8-bit mono	(m8)
// 8-bit stereo (s8)
//16-bit mono	(m16)
//16-bit stereo	(s16)
//

#ifndef _SAMPLE_TYPES_H_
#define _SAMPLE_TYPES_H_
#include <limits> // for SHRT_MIN etc.
//#include <float.h> // for float limits

//
// define mono sampletypes
//
typedef unsigned char sampletype_m8;
typedef short sampletype_m16;
typedef long accumulatortype;
typedef double sampletype_md;
typedef float sampletype_mf;

//
// sampletraits give some info about a given sampletype
// currently this info consists of the 'accumulatortype'
// i.e. a type which can contain the sum of a large number
// (32k) of samples
//
template< class sampletype>
struct sampletraits
{

	template <typename other_type>
	struct accumulator_gen
	{
		typedef typename sampletype:: template accumulator_gen<other_type>::type type;
	};

	template <typename other_type>
	struct channel_gen
	{
		typedef typename sampletype:: template channel_gen<other_type>::type type;
	};

	typedef typename channel_gen<void>::type channel_type;
	typedef typename accumulator_gen<void>::type accumulator_type;

	template< typename destination_type>
	static void extract_channel( const sampletype &sample, destination_type &destination, short channel)
	{
		return sampletype::extract_channel( sample, destination, channel);
	}

	template <typename numeric_type>
		struct foreach_channel_type
		{
			typedef typename sampletype:: template foreach_channel_type<numeric_type>::type type;
		};

	template <typename sourcetype>
	static typename foreach_channel_type<sourcetype>::type expand_to_channels( const sourcetype &source)
	{
		return sampletype::expand_to_channels( source);
	}

	static inline sampletype get_min()
	{
		return sampletype::get_min();
	}
	static inline sampletype get_max()
	{
		return sampletype::get_max();
	}
	static inline sampletype get_middle()
	{
		return sampletype::get_middle();
	}
	static inline sampletype negate( const sampletype &input)
	{
		return sampletype::negate( input);
	}

	static inline short get_num_channels()
	{
		return sampletype::get_num_channels();
	}

	template <typename operator_type>
	static inline void apply_to_all_channels( operator_type &op, sampletype &sample)
	{
		sampletype::apply_to_all_channels( op, sample);
	}
};

template <typename simple_type, typename accumulatortype>
struct builtin_type_sampletraits
{

	typedef simple_type sampletype;
	typedef sampletype channel_type;
	typedef accumulatortype accumulator_type;

	template <typename other_type>
	struct channel_gen
	{
		typedef channel_type type;
	};

	template< typename destination_type>
	static void extract_channel( const sampletype &sample, destination_type &destination, short channel)
	{
		destination = sample;
	}

	template <typename other_type>
		struct accumulator_gen
		{
			typedef accumulatortype type;
		};

	template <typename numeric_type>
	struct foreach_channel_type
	{
		typedef numeric_type type;
	};

	template< typename sourcetype>
	static inline sourcetype expand_to_channels( const sourcetype &source)
	{
		return source;
	}

	template <typename operator_type>
	static inline void apply_to_all_channels( operator_type &op, sampletype &sample)
	{
		op(sample);
	}

	static inline sampletype get_min()
	{
		return std::numeric_limits< sampletype>::min();
	}

	static inline sampletype get_max()
	{
		return std::numeric_limits< sampletype>::max();
	}

	static inline sampletype get_middle()
	{
		return 0;
	}

	static inline sampletype negate( const sampletype_m16 &input)
	{
		return input==get_min()?get_max():-input;
	}

	static inline short get_num_channels()
	{
		return 1;
	}
};

template <> struct sampletraits< sampletype_m16> : public builtin_type_sampletraits< sampletype_m16, long>
{
};

template <> struct sampletraits< sampletype_m8> : public builtin_type_sampletraits< sampletype_m8, long>
{
	static inline sampletype_m8 get_middle()
	{
		return 128;
	}

	static inline sampletype negate( const sampletype &input)
	{
		return 255-input;
	}
};

template <> struct sampletraits< float> : public builtin_type_sampletraits< float, double>
{
	static inline float negate( const sampletype &input)
	{
		return -input;
	}

	static inline sampletype get_min()
	{
		return -1.0;
	}

	static inline sampletype get_max()
	{
		return 1.0;
	}

};

template <> struct sampletraits< double>: public builtin_type_sampletraits< double, double>
{
	static inline double negate( const sampletype &input)
	{
		return -input;
	}

	static inline sampletype get_min()
	{
		return -1.0;
	}

	static inline sampletype get_max()
	{
		return 1.0;
	}


};


#pragma pack(push,1)
struct sampletype_m24
{
public:
	unsigned short  ls_part;
	char			ms_part;


public:
	typedef long accumulatortype;
	typedef sampletype_m24 this_type;

	template <typename other_type>
		struct accumulator_gen
		{
			typedef accumulatortype type;
		};

	template <typename other_type>
	struct channel_gen
	{
		typedef this_type type;
	};

	template<typename destination_type>
	static void extract_channel( const this_type &sample, destination_type &destination, short channel)
	{
		destination = sample;
	}


	template <typename numeric_type>
	struct foreach_channel_type
	{
		typedef numeric_type type;
	};

	template< typename sourcetype>
	static inline sourcetype expand_to_channels( const sourcetype &source)
	{
		return source;
	}

	template <typename operator_type>
	static inline void apply_to_all_channels( operator_type &op, sampletype_m24 &sample)
	{
		op(sample);
	}

	static unsigned short ls_from_int( int in)
	{
		return in & 0xffff;
	}

	static char ms_from_int( int in)
	{
		return (in>>16) & 0xff;
	}

	operator int() const
	{
		return (static_cast<int>(ms_part) << 16 ) + ls_part;
	}

	inline sampletype_m24( long rhs)
		:ls_part( ls_from_int( rhs)),
		ms_part( ms_from_int( rhs))
	{
		/*nop*/
	};

	inline sampletype_m24( const sampletype_m24 &rhs)
		: ls_part( rhs.ls_part),
		ms_part( rhs.ms_part)
	{
		// nop
	}

	inline sampletype_m24( double rhs)
		: ls_part( ls_from_int( int(rhs))),
		  ms_part( ms_from_int( int(rhs)))
	{
		// nop
	}
	inline sampletype_m24()
	{
		/*nop*/
	};

	sampletype_m24( char ms, unsigned short ls)
		: ms_part( ms),
		ls_part( ls)
	{
		// nop
	}

	inline bool operator<( const sampletype_m24 &rhs) const
	{
		return (int)*this < (int)rhs;
	}

	inline const sampletype_m24 operator+( const sampletype_m24 &rhs) const
	{
		return (long)rhs + (long)*this;
	}

	inline sampletype_m24 &operator+=( const sampletype_m24 &rhs)
	{
		*this = *this + rhs;
		return *this;
	}

	inline const sampletype_m24 &operator*=( const long mult)
	{
		*this = ((long) *this) * mult;
		return *this;
	}

	inline const sampletype_m24 &operator*=( float fmult)
	{
		*this = (long)(((long) *this) * fmult);
		return *this;
	}

	inline const sampletype_m24 &operator*=( double fmult)
	{
		*this = (long)(((long) *this) * fmult);
		return *this;
	}

	inline const sampletype_m24 &operator/=( long div)
	{
		*this = *this / div;
		return *this;
	}

	inline const sampletype_m24 operator/( const long &rhs) const
	{
		return ((int)( *this)) / rhs;
	}

	inline const sampletype_m24 &operator/=( short div)
	{
		*this = *this / div;
		return *this;
	}

	inline const sampletype_m24 operator/( const short &rhs) const
	{
		return ((long)( *this)) / rhs;
	}


	static inline sampletype_m24 get_min()
	{
		return sampletype_m24( -128, 0);
	}

	static inline sampletype_m24 get_max()
	{
		return sampletype_m24( 127, 0xffff);
	}

	static inline sampletype_m24 get_middle()
	{
		return sampletype_m24( 0, 0);
	}

	static inline sampletype_m24 negate( const sampletype_m24 &rhs)
	{
		return -((long)rhs);
	}

	static inline short get_num_channels()
	{
		return 1;
	}
};
#pragma pack(pop)



//
// definition of stereo sample types from their mono sample types
//
template< class MonoSample>
struct StereoSample
{
public:
	MonoSample m_left;
	MonoSample m_right;


	template <typename other>
	struct accumulator_gen
	{
		typedef StereoSample< typename sampletraits<MonoSample>::template accumulator_gen<other>::type> type;
	};

	template< typename other>
	struct channel_gen
	{
		typedef typename sampletraits<MonoSample>::template channel_gen<other>::type type;
	};

	typedef MonoSample monosample;
	typedef StereoSample<MonoSample> this_type;

	template <typename numeric_type>
	struct foreach_channel_type
	{
		typedef StereoSample<typename sampletraits<MonoSample>::template foreach_channel_type<numeric_type>::type> type;
	};

	template< typename destination_type>
	static void extract_channel( const this_type &sample, destination_type &destination, short channel)
	{
		if (channel < sampletraits<MonoSample>::get_num_channels())
		{
			sampletraits<MonoSample>::extract_channel( sample.m_left, destination, channel);
		}
		else
		{
			sampletraits<MonoSample>::extract_channel(
				sample.m_right,
				destination,
				channel - sampletraits<MonoSample>::get_num_channels());
		}
	}

	template< typename sourcetype>
		inline static typename foreach_channel_type<sourcetype>::type expand_to_channels( const sourcetype &source)
	{
		return typename foreach_channel_type<sourcetype>::type( sampletraits<MonoSample>::expand_to_channels( source), sampletraits<MonoSample>::expand_to_channels( source));
	}



	template <typename operator_type>
	static inline void apply_to_all_channels( operator_type &op, this_type &sample)
	{
		sampletraits<MonoSample>::apply_to_all_channels( op, sample.m_left);
		sampletraits<MonoSample>::apply_to_all_channels( op, sample.m_right);
	}

public:
	inline const MonoSample left() const { return m_left;}
	inline const MonoSample right() const {return m_right;}

	inline StereoSample( const MonoSample &left, const MonoSample &right)
		:m_left( left), m_right( right)
	{
		/*nop*/
	};

	inline StereoSample()
	{
		/*nop*/
	};
/*
	inline StereoSample( const accumulatortype &rhs)
	{
		m_left = static_cast<MonoSample>( rhs.m_left);
		m_right = static_cast<MonoSample>( rhs.m_right);
	}
*/
	inline StereoSample( const MonoSample &rhs)
		: m_left( rhs),
		m_right( rhs)
	{
	}

	template <typename sampletype>
		inline StereoSample( const StereoSample< sampletype> &s)
		: m_left( (MonoSample)s.left()), m_right( (MonoSample)s.right()) {};

	template <typename sampletype>
		inline operator StereoSample< sampletype>()
	{
		return StereoSample<sampletype>( (sampletype)m_left, (sampletype)m_right);
	}

	inline bool operator<( const this_type &rhs) const
	{
		return m_left + m_right < rhs.m_left + rhs.m_right;
	}

	template<typename other_type>
	inline StereoSample<MonoSample> &operator+=( const StereoSample<other_type> &rhs)
	{
		m_left += rhs.m_left;
		m_right += rhs.m_right;
		return *this;
	}

	template<typename other_type>
	inline StereoSample<MonoSample> &operator-=( const StereoSample<other_type> &rhs)
	{
		m_left -= rhs.m_left;
		m_right -= rhs.m_right;
		return *this;
	}

	template< typename other_type>
	inline const StereoSample< MonoSample> operator+( const StereoSample< other_type> &rhs) const
	{
		return StereoSample< MonoSample>( *this) += rhs;
	}

	template< typename other_type>
	inline const StereoSample< MonoSample> operator-( const StereoSample< other_type> &rhs) const
	{
		return StereoSample< MonoSample>( *this) -= rhs;
	}

	template< typename scalar_type>
	inline const StereoSample< MonoSample> &operator*=( scalar_type mult)
	{
		m_left *= mult;
		m_right *= mult;
		return *this;
	}

	template< typename scalar_type>
	inline const StereoSample< MonoSample> &operator/=( scalar_type div)
	{
		m_left /= div;
		m_right /= div;
		return *this;
	}

	template< typename scalar_type>
	inline const StereoSample< MonoSample> operator/( scalar_type rhs) const
	{
		return StereoSample< MonoSample>( *this) /= rhs;
	}

	template <typename scalar_type>
	inline const StereoSample<MonoSample> operator*( scalar_type rhs) const
	{
		return StereoSample< MonoSample>( *this) *= rhs;
	}

	inline operator MonoSample()
	{
		return (m_left + m_right) / 2;
	}

	static inline this_type get_min()
	{
		return this_type( sampletraits<MonoSample>::get_min(), sampletraits<MonoSample>::get_min());
	}
	static inline this_type get_max()
	{
		return this_type( sampletraits<MonoSample>::get_max(), sampletraits<MonoSample>::get_max());
	}
	static inline this_type get_middle()
	{
		return this_type( sampletraits<MonoSample>::get_middle(), sampletraits<MonoSample>::get_middle());
	}
	static inline this_type negate( const this_type &rhs)
	{
		return this_type( sampletraits<MonoSample>::negate(rhs.left()), sampletraits<MonoSample>::negate( rhs.right()));
	}

	static inline short get_num_channels()
	{
		return 2 * sampletraits<MonoSample>::get_num_channels();
	}
};

// create a stereo sample, given monosamples of some type.
template< typename sampletype>
StereoSample< sampletype> make_stereo_sample( const sampletype &left, const sampletype &right)
{
	return StereoSample< sampletype>( left, right);
}


template< typename sampletype>
StereoSample< sampletype> abs(  const StereoSample<sampletype> &rhs)
{
	return make_stereo_sample( abs( rhs.m_left), abs( rhs.m_right));
}
//
// make the stereo sample types
//
typedef struct StereoSample<sampletype_m8> sampletype_s8;
typedef struct StereoSample<sampletype_m16> sampletype_s16;
typedef struct StereoSample<sampletype_m24> sampletype_s24;
typedef struct StereoSample<sampletype_md> sampletype_sd;
typedef struct StereoSample<sampletype_mf> sampletype_sf;


//
// define quad types
//
typedef struct StereoSample<sampletype_s8> sampletype_q8;
typedef struct StereoSample<sampletype_s16> sampletype_q16;
typedef struct StereoSample<sampletype_s24> sampletype_q24;
typedef struct StereoSample<sampletype_sd> sampletype_qd;
typedef struct StereoSample<sampletype_sf> sampletype_qf;

template<typename sampletype>
inline double normalize_sample( const sampletype &input)
{
	typedef sampletraits< sampletype> traits;
	const double inp = input;
	const double mid = traits::get_middle();
	const double max = traits::get_max();
	const double min = traits::get_min();

	return (inp>mid)?(inp-mid)/(max-mid):-((mid - inp) / (mid - min));
}

template<>
inline double normalize_sample( const double &input)
{
	return input;
}

template<typename sampletype>
inline typename sampletraits< StereoSample< sampletype> >::template foreach_channel_type< double>::type
	normalize_sample( const StereoSample<sampletype> &input)
{
	return make_stereo_sample(
			normalize_sample( input.m_left),
			normalize_sample( input.m_right)
		);
}

/*
template<typename sampletype>
inline void denormalize_sample( double in, sampletype &out)
{
	typedef sampletraits<sampletype> traits_type;
	const double min = traits_type::get_min();
	const double mid = traits_type::get_middle();
	const double max = traits_type::get_max();

	if (in > mid)
	{
		out = sampletype( in * (max - mid) + mid);
	}
	else
	{
		out = sampletype( in * (mid - min) + mid);
	}

}
*/


template<typename sampletype>
inline void denormalize_sample( typename sampletraits<sampletype>::template foreach_channel_type<double>::type in, sampletype &out)
{
	typedef sampletraits<sampletype> traits_type;
	typedef sampletraits< typename traits_type::channel_type> channel_traits;


	static const double temp = channel_traits::get_max() - channel_traits::get_middle();
	static const sampletype middle = traits_type::expand_to_channels( channel_traits::get_middle());

	in *= temp;
	in -= middle;
	out = static_cast<sampletype>( in);


}

inline void denormalize_sample( double in, double &out)
{
	out = in;
}

/*
inline void denormalize_sample( sampletraits<double>::foreach_channel_type<double>::type in, double &out)
{
	out = in;
}
*/

template< typename sampletype>
inline sampletype min_sample( const sampletype &lhs, const sampletype &rhs)
{
	return ( lhs < rhs) ? lhs : rhs;
}

template< typename monosampletype>
inline StereoSample< monosampletype> min_sample( const StereoSample<monosampletype> &lhs,
													const StereoSample< monosampletype> &rhs)
{
	return StereoSample< monosampletype>(
			min_sample( lhs.m_left, rhs.m_left),
			min_sample( lhs.m_right, rhs.m_right)
		);
}

template< typename sampletype>
inline sampletype max_sample( const sampletype &lhs, const sampletype &rhs)
{
	return (lhs < rhs) ? rhs : lhs;
}

template< typename monosampletype>
inline StereoSample< monosampletype> max_sample( const StereoSample<monosampletype> &lhs,
													const StereoSample< monosampletype> &rhs)
{
	return StereoSample< monosampletype>(
			max_sample( lhs.m_left, rhs.m_left),
			max_sample( lhs.m_right, rhs.m_right)
		);
}

#endif
