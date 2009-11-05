#if !defined(MIDI_EVENT_VISITOR_HPP)
#define MIDI_EVENT_VISITOR_HPP

#include <boost/variant/apply_visitor.hpp>
#include "midi_event_types.hpp"

namespace events
{

	template<typename Derived>
	struct visitor : public boost::static_visitor<>
	{
		Derived &derived()
		{
			return *static_cast<Derived*>(this);
		}

		const Derived &derived() const
		{
			return *static_cast<Derived*>(this);
		}

		void operator()( const any &event)
		{
			boost::apply_visitor( derived(), event);
		}

		void operator()( const channel_event &event)
		{
			current_channel = event.channel;
			boost::apply_visitor( derived(), event.event);
		}

		void operator()( const meta &)
		{
		}

		void operator()( const sysex &)
		{
		}

		void operator()( const note_on &){}
		void operator()( const note_off &){}
		void operator()( const note_aftertouch &){}
		void operator()( const controller &){}
		void operator()( const program_change &){}
		void operator()( const channel_aftertouch &){}
		void operator()( const pitch_bend &){}

		unsigned short current_channel;
	};

	template<typename Derived>
	struct timed_visitor : public visitor<Derived>
	{
		timed_visitor()
			:current_time(0)
		{

		}

		using events::visitor<Derived>::operator();
		void operator()( const timed_midi_event &event)
		{
			current_time += event.delta_time;
			derived()( event.event);
		}

		void reset()
		{
			current_time = 0;
		}

		size_t current_time;
	};

} // namespace events


#endif //MIDI_EVENT_VISITOR_HPP