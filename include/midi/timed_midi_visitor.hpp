#if !defined( TIMED_MIDI_VISITOR_HPP)
#define TIMED_MIDI_VISITOR_HPP

#include "midi_file.hpp"

namespace events
{
	template<typename Derived>
	struct timed_visitor : public visitor<Derived>
	{
		timed_visitor( const midi_header &h)
			:current_time(0.0), time_step(1.0), ignore_bpm(false), bpm( 120)
		{
			if (h.division & 0x8000)
			{
				int fps_raw = (h.division & 0x7f00) >> 8;
				double fps = fps_raw == 29?29.97:fps_raw;
				time_step = fps * (h.division & 0x00ff);
				ignore_bpm = true;
			}
			else
			{
				time_step = ((double)bpm/h.division)/60;
			}
		}

		using visitor<Derived>::operator();

		void operator()( const timed_midi_event &event)
		{
			current_time += (event.delta_time * time_step);
			derived()( event.event);
		}

		void reset()
		{
			current_time = 0.0;
		}

		double	current_time;
		double  time_step;
		bool	ignore_bpm;
		unsigned int bpm;
	};


}
#endif //TIMED_MIDI_VISITOR_HPP