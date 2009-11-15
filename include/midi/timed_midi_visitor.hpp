#if !defined( TIMED_MIDI_VISITOR_HPP)
#define TIMED_MIDI_VISITOR_HPP
#include "midi_event_visitor.hpp"
#include "midi_file.hpp"

namespace events
{
	template<typename Derived>
	struct timed_visitor : public visitor<Derived>
	{
		timed_visitor( const midi_header &h)
			:current_time(0.0), time_step(1.0), ignore_bpm(false), division( h.division)
		{
			if (h.division & 0x8000)
			{
				int fps_raw = (h.division & 0x7f00) >> 8;
				double fps = (fps_raw == 29)?29.97:fps_raw;
				time_step = fps * (h.division & 0x00ff);
				ignore_bpm = true;
			}
			else
			{
                // assume 120 bpm (0.5s/beat) at start
				time_step = .5/division;
			}
		}

		using visitor<Derived>::operator();

		void operator()( const timed_midi_event &event)
		{
			current_time += (event.delta_time * time_step);
			derived()( event.event);
		}

        void operator() (const meta &event)
        {
            // react on tempo changes
            if (event.type == 81 && event.bytes.size() == 3 && !ignore_bpm)
            {
                unsigned microseconds_per_quarter_note = (event.bytes[0] << 16) + (event.bytes[1] << 8) + event.bytes[2];
                time_step = (microseconds_per_quarter_note / 1000000.0) / division;
            }
        }

		void reset()
		{
			current_time = 0.0;
		}

		double	current_time;
		double  time_step;
		bool	ignore_bpm;
		unsigned int division;
	};


}
#endif //TIMED_MIDI_VISITOR_HPP