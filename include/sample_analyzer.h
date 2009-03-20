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

