template< typename sampletype>
class amplifier_mutator
{
private:
	struct amplify_op
	{
		template< typename actual_type>
		void operator()( actual_type &sample)
		{
			sample = actual_type( sample * m_factor);
		}

		template<>
			void operator()<sampletype_m8>( sampletype_m8 &sample)
		{
			sample = static_cast<sampletype_m8>((static_cast<int>(sample) - sampletraits<sampletype_m8>::get_middle()) * m_factor) + sampletraits<sampletype_m8>::get_middle();
		}

		amplify_op( double factor)
			: m_factor( factor)
		{
		}

		amplify_op()
			: m_factor( 0.0)
		{
		}


		double m_factor;
	};

public:

	typedef sampletype sample_type;
	typedef bool result_information_type;

	void MutateHeader( const stream_header &)
	{
		// nop
	}

	void Mutate( sampletype *sample)
	{
		sampletraits<sampletype>::apply_to_all_channels( m_operation, *sample);
	}


	result_information_type GetResult()
	{
		return  true;
	}

	void SetFactor( double factor)
	{
		m_operation = amplify_op( factor);
	}

private:
	amplify_op m_operation;
};

template< typename sample_type>
struct amplifier : public uniform_block_converter< mutator_adapter< amplifier_mutator< sample_type> > >
{
	amplifier( double factor)
	{
		m_converter.SetFactor( factor);
	}
};
