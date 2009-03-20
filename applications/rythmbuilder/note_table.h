class note_table
{
private:
	const std::string m_notes_directory;
	const size_t m_length_ms;
	double m_tune_delta;

public:
	note_table( const std::string &notes_directory, size_t length_ms)
		:m_notes_directory( notes_directory),
		m_length_ms( length_ms),
		m_tune_delta(0)
	{
		// nop
	}

	// detune with tune_delta semitones
	void detune( double tune_delta)
	{
		m_tune_delta = tune_delta;
	}

	block_producer *get_note( std::string note_name)
	{
		block_producer *result_ptr = 0;

		std::string full_name = m_notes_directory + "\\" + note_name + ".wav";
		result_ptr = filefactory::GetBlockProducer( full_name.c_str());
		
		if (result_ptr)
		{
			// slightly detune the note.
			result_ptr = detune_note( result_ptr, m_tune_delta);

			stream_header h;
			result_ptr->GetStreamHeader( h);
			size_t length_ms = h.numframes * 1000 / h.samplerate;

			// add a random delay of 0-10 ms so that mixed samples are less 'synchronized'.
			// calculate the delay and optional padding in seconds.
			// padding must be added if the sum of the lengths of the delay and the original
			// note are smaller than the requested sample length.
			size_t delay_ms = rand() * 20 / RAND_MAX ; // ms delay, up to 10 ms
			double delay_s = double( delay_ms) / 1000.0; // sec delay
			double added_size_s = 0.0; // padding
			
			// add padding if the delay combined with  the original note length 
			// is smaller than the requested size
			if (delay_ms + length_ms < m_length_ms)
			{
				added_size_s = double(m_length_ms - delay_ms - length_ms) / 1000.0;
			}

			// now create the actually delayed sample (with optional padding)
			sample_object_factory *fact = factory_factory::GetSampleFactory( h);
			block_mutator *mutator_ptr = fact->GetDelay( delay_s , added_size_s);
			mutator_ptr->LinkTo( result_ptr);
			result_ptr = mutator_ptr;
			delete fact;

			if ( delay_ms + length_ms > m_length_ms)
			{
				// if the result of the delay plus original note is longer than 
				// the requested duration, we cut some samples at the end.
				cut_mutator *cutter = new cut_mutator();
				cutter->LinkTo( result_ptr);
				cutter->SetCut( 0, static_cast<long>((m_length_ms * h.samplerate) / 1000));
				result_ptr = cutter;
			}

		}
		return result_ptr;
	}

private:
	static block_producer *detune_note( block_producer *producer_ptr, double detune_delta)
	{
		// get some easy cases out of the way.
		if (!producer_ptr) return 0;
		if (detune_delta == 0.0) return producer_ptr;


		stream_header h;
		producer_ptr->GetStreamHeader( h);

		unsigned short new_samplerate = static_cast< unsigned short>(double( h.samplerate) * pow( 2.0, (-detune_delta/12.0)));
		sample_object_factory *fact = factory_factory::GetSampleFactory( h);
		block_mutator *my_resampler =  fact->GetResampler( new_samplerate, unsigned short( h.samplerate), 
										true); // true means do not change actual samplerate
		my_resampler->LinkTo( producer_ptr);
		return my_resampler;
	}
};
