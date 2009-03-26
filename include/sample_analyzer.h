#if !defined( SAMPLE_ANALYZER_H)
#define SAMPLE_ANALYZER_H

#include "samplebl.h"

struct sound_analysis
{
	double avg;
	double max;
	double min;
	double norm;
};

struct sample_analyzer : public block_mutator
{
	virtual sound_analysis GetResult() = 0;
	virtual void Reset() = 0;
};

#endif //SAMPLE_ANALYZER_H
