//////////////////////////////////////////////
// stereo panning
// A stereo pan object receives stereo samples and performs panning
// on them by suppressing either the left signal (pan right) or the
// right signal (pan left).
//
template< typename mono_type>
class stereo_pan_mutator
{
public:
	typedef StereoSample< mono_type> sample_type;
	typedef int result_information_type;

	stereo_pan_mutator( )
		: m_pan_value( 0)
	{
		
	}

	void set_pan( short pan_value)
	{
		m_pan_value = pan_value;
	}

	inline void MutateHeader( stream_header &h)
	{
		// nop
	}

	inline void Mutate( sample_type *p_s)
	{
		if (m_pan_value< 0)
		{
			p_s->m_right = fixed_damp( 32768 + m_pan_value, p_s->m_right);
		}
		else 
		{
			p_s->m_left = fixed_damp( 32767 - m_pan_value, p_s->m_left); 
		}
	}

	inline int GetResult() {return 0;}

private:
	short m_pan_value;
};

template< typename mono_type>
class stereo_pan : public uniform_block_mutator< stereo_pan_mutator< mono_type> >
{
public:
	stereo_pan( short pan_value)
	{
		m_sample_mutator.set_pan( pan_value);
	};
};
