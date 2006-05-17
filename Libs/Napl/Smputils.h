////////////////////////////////////////////////////////////////////////////////
//
// smputils.h - sample utility functions
//
#ifndef _SAMPLE_UTILS_H
#define _SAMPLE_UTILS_H

// balance delivers a sample that is a balanced average of s1 and s2,
// where s1:s2 == q1:q2
	template< typename sampletype>
	static inline const sampletype balance( 
		const unsigned short q1, 
		const unsigned short q2, 
		const sampletype &s1, 
		const sampletype &s2)
	{
		typedef sampletraits<sampletype>::accumulator_gen<void>::type accumulatortype;
		return sampletype(
			(accumulatortype( s1) * ( (long)q1) +
			accumulatortype( s2) * ( (long)q2)) /
			(long)(q1 + q2)
			);
		/*
		 return sample_cast< sampletype, sampletraits< sampletype>::accumulatortype>(
			 (sample_cast<sampletraits< sampletype>::accumulatortype, sampletype>(s1) * q1 + (sample_cast<sampletraits< sampletype>::accumulatortype, sampletype>(s2) * q2))/ (q1 + q2));
		*/
	}

//
// fixed_balance uses a scale of 0 - 32768, where 0 means s1 and no s2, 
// 32768 means s2 and no s1, and every value in between means a mix of 
// s1 and s2.
//
	template< typename sampletype>
	static inline const sampletype fixed_balance( 
		const unsigned short q1, 
		const sampletype &s1, 
		const sampletype &s2)
	{
		typedef sampletraits<sampletype>::accumulator_gen<void>::type accumulatortype;
		return sampletype(
			(accumulatortype( s1) * ( (long)q1) +
			accumulatortype( s2) * ( (long)(0x8000 - q1))) /
			(long)(0x8000)
			);
	}

	// 
	// fixed_damp is a fixed-point 'damper'. it takes a factor between 0 and 65535
	// and a sample and multiplies the sample with (factor/32768)
	//
	template< typename sampletype>
		static inline sampletype fixed_damp( unsigned short factor, const sampletype &sample)
	{
		typedef sampletraits<sampletype>::accumulator_gen<void>::type accumulatortype;
		return sampletype( 
			(accumulatortype( sample) * factor) / (long)(0x8000)
			);
	}


#endif 
