#if !defined( MIDI_MULTIPLEXER_HPP)
#define MIDI_MULTIPLEXER_HPP
#include <utility> // for std::pair
#include <boost/function.hpp>
#include "midi_event_types.hpp"
#include "midi_file.hpp" // for midi_track

/// This class accepts a range of midi tracks and will offer the midi events in 
/// these tracks in chronological order to any visitor provided.
class midi_multiplexer
{
	typedef midi_file::tracks_type	   tracks_type;
	typedef midi_track::const_iterator track_iterator;
	struct track_range
	{
		unsigned int offset;
		track_iterator begin;
		track_iterator end;

		track_range( track_iterator b, track_iterator e)
			: offset(0), begin( b), end( e)
		{
		}

		/// returns the delta time of the first event
		/// in the range, minus offset
		/// precondition: range is not empty.
		unsigned int time() const
		{
			return begin->delta_time - offset;
		}

		bool empty() const
		{
			return begin == end;
		}
	};
	typedef std::vector< track_range>  ranges_vector;


public:
	midi_multiplexer( const tracks_type &tracks)
	{
		for (tracks_type::const_iterator i = tracks.begin(); i != tracks.end();++i)
		{
			if (i->begin() != i->end())
			{
				ranges.push_back( track_range( i->begin(), i->end()));
			}
		}
	}

	/// find the range with the earliest timed_midi_event.
	/// precondition: ranges is not empty
	ranges_vector::iterator find_earliest()
	{
		ranges_vector::iterator i = ranges.begin();
		ranges_vector::iterator earliest = i;
		unsigned int earliest_time = i->time();

		for ( /*nop*/; i != ranges.end(); ++i)
		{
			if (i->time() < earliest_time)
			{
				earliest = i;
				earliest_time = i->time();
			}
		}

		return earliest;
	}

	/// subtract an offset from the first element of every range.
	/// precondition: all ranges are non-empty and all offsets are at least 'offset'
	void adapt_offsets( unsigned int offset)
	{
		for (ranges_vector::iterator i = ranges.begin(); i != ranges.end(); ++i)
		{
			i->offset += offset;
		}
	}


	void accept( boost::function< void ( const events::timed_midi_event &)> v)
	{
		while (!ranges.empty())
		{
			// find the range with the earliest event
			ranges_vector::iterator 
				earliest = find_earliest();

			// invoke the visitor with the next event
			events::timed_midi_event e = *earliest->begin;
			e.delta_time = earliest->time();

			v( e);
			
			// now adapt all other midi event delta times.
			adapt_offsets( earliest->time());

			++earliest->begin;
			earliest->offset = 0;


			// while the range is nog empty and there are events in this track that are simultaneous
			// give all these events to the visitor. This keeps simultaneous events in one track 
			// together.
			while (
					!earliest->empty()
				&&	earliest->begin->delta_time == 0)
			{
				v( *earliest->begin);
				++earliest->begin;
			}

			// remove the track if we've exhausted all events.
			if (earliest->empty())
			{
				ranges.erase( earliest);
			}
		}
		

	}

private:
	ranges_vector	ranges;
};

#endif //MIDI_MULTIPLEXER_HPP