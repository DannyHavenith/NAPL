#if !defined( GENERAL_RESAMPLER_H)
#define GENERAL_RESAMPLER_H

class general_resampler : public block_mutator, public block_owner
{
public:
	virtual void Init(unsigned long outfreq, unsigned long infreq, bool lie) = 0;
};

#endif
