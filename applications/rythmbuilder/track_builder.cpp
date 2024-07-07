#include "instrument.h"
#include "napl.h"
#include "samplebl.h"
#include "simpops.h"
#include "track_builder.h"

#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

using namespace std;

namespace
{
    using block_producer_ptr = std::shared_ptr< block_producer>;

    block_producer_ptr Concatenate( block_producer_ptr left, block_producer_ptr right)
    {
        auto paster = std::make_shared<owning_paste_mutator>( std::make_unique<paste_mutator>());

        paster->LinkTo( left, 0);
        paster->LinkTo( right, 1);

        return paster;
    }

    block_producer_ptr Add( block_producer_ptr left, block_producer_ptr right)
    {
            stream_header h;
            left->GetStreamHeader( h);
            std::unique_ptr<sample_object_factory> factory_ptr{ factory_factory::GetSampleFactory( h)};
            auto adder = std::make_shared<owning_binary_block_processor>(std::unique_ptr<binary_block_processor>{factory_ptr->GetAdder()});
            adder->LinkTo( left, right);

            return adder;
    }

    //
    // join all samples in a range into a single sequence.
    // Doesn't really need RAIterators, but is a lot slower otherwise.
    //
    template< class RAIterator, class Op>
    block_producer_ptr join_sounds( RAIterator begin, const RAIterator &end, Op op)
    {
        size_t dist = std::distance( begin, end);

        if (dist == 0)
        {
            return 0;
        }

        if (dist == 1)
        {
            return *begin;
        }

        return op(
                join_sounds( begin, begin+dist/2, op),
                join_sounds( begin+dist/2, end, op)
                );

    }

    // spread the sound in the iterator range over the stereo space (so that the first one is
    // on the utter left channel and the last on the utter right.
    //
    template<
        typename iterator_t,
        typename output_iterator>
        void spread_bars(
            iterator_t begin, iterator_t end,
            output_iterator destination)
    {
        if (begin == end) return;

        size_t count = std::distance( begin, end);

        stream_header h;
        (*begin)->GetStreamHeader( h);
        sample_object_factory *factory_ptr = factory_factory::GetSampleFactory( h);

        if (count > 1)
        {
            short step = 2 * (32767 / static_cast<short>(count-1));
            short pan_position = -32768;
            for (iterator_t current = begin; current != end; ++current)
            {
                std::shared_ptr<block_mutator> panner{ factory_ptr->GetPan( pan_position)};
                panner->LinkTo( current->get());
                *destination++ = panner;
                pan_position += step;
            }
        }
        else
        {
            *destination = *begin;
        }
    }


}

track_builder::track_builder(
    instrument_factory &instruments_,
    const std::string &default_name,
    std::ostream &logging_stream)
    : note_seconds(.25),
    instruments( instruments_),
    last_measure_index(0),
    anacrusis_index(0),
    current_note{},
    track_name( default_name),
    logging_stream( logging_stream)
{
};

void track_builder::start_track( const string &rythm, const string &section, int bpm, const string &comment)
{
    note_seconds = 60.0/bpm;
    track_name = rythm.empty()?string("track"): rythm;
    section_name = section;
}

void track_builder::cleanup()
{
    last_measure_index = 0;
    anacrusis_index = 0;

    end_bar();

    if (!track.empty())
    {
        emit_track();
        track.clear();
    }
}

void write_file( const std::string &filename, block_producer *p)
{
    block_sink *file_writer = filefactory::GetBlockSink( filename.c_str());

    file_writer->LinkTo( p);

    //
    // write to file...
    file_writer->Start();

}

void track_builder::emit_track()
{

    sound_vector bars( track.size());

    // spread over stereo space.
    spread_bars( track.begin(), track.end(), bars.begin());

    // add all bars
    auto result =
        join_sounds( bars.begin(), bars.end(), Add);

    std::string filename = track_name;
    if (!section_name.empty())
    {
        filename += "_" + section_name;
    }

    filename += ".wav";

    std::unique_ptr<block_sink> file_writer{ filefactory::GetBlockSink( filename.c_str())};
    if (!file_writer)
    {
        throw std::runtime_error( "could not open file " + filename + " for writing, is it open in another application?");
    }

    file_writer->LinkTo( result.get());

    //
    // write to file...
    file_writer->Start();


}

//
// start a new bar.
// precondition: current_bar.empty() == true
//
void track_builder::start_bar( const string &bar_name, const string &instrument_name)
{
    string instrument = instrument_name;

    if (instrument.empty())
    {
        instrument = bar_name;
    }

    if (instrument.empty())
    {
        instrument = "djembe1";
    }

    current_instrument = instruments.get_instrument( instrument);
}


void track_builder::start_nlet( int numerator, int denominator)
{
    tempo.push( note_seconds);

    if (denominator == 0) denominator = 1;
    if (numerator == 0) numerator = 1;

    //
    // this  may look counter-intuitive: if we encounter an nlet of the
    // form n/m(...), for example 3/2(...), this means that the nlet itself takes 2 beats
    // for every three notes encountered. So note duration becomes, in this case 2/3 of the original
    // duration.
    //
    // a typical triplet would thus become 3/2(...).
    //
    note_seconds = (note_seconds * denominator) / numerator;
}



void track_builder::new_measure()
{
    push_note();
    last_measure_index = notes.size();
}

void track_builder::mark_anacrusis()
{
    push_note();
    anacrusis_index = notes.size();
}

double track_builder::total_duration(
    note_vector::const_iterator begin,
    note_vector::const_iterator end)
{
    return std::accumulate( begin, end, 0.0, []( double sum, const note &n)
    {
        return sum + n.seconds;
    });
}

void track_builder::repeat( int count)
{

    if (count == 0) return;

    push_note();

    //
    // create a copy of the notes to repeat, because we're going to invalidate all iterators by
    // performing a copy into a vector.
    //
    auto measure_begin =
        (last_measure_index != notes.size()) ?
                (notes.begin() + last_measure_index)
            :   notes.begin();

    const note_vector measure( measure_begin, notes.end());
    const auto duration = total_duration( measure.begin(), measure.end());
    const auto local_anacrusis_index = anacrusis_index - std::distance(  notes.begin(), measure_begin);
    notes.erase( measure_begin, notes.end());

    note_vector last_fragment;
    if (anacrusis_index != 0 and anacrusis_index < notes.size() + measure.size())
    {
        const auto anacrusis_duration = total_duration( measure.begin() + local_anacrusis_index, measure.end());
        note_vector anacrusis;
        anacrusis.push_back( { "", duration - anacrusis_duration});
        anacrusis.insert( anacrusis.end(), measure.begin() + local_anacrusis_index, measure.end());

        last_fragment.assign( measure.begin(), measure.begin() + local_anacrusis_index);
        last_fragment.push_back( { "", anacrusis_duration});
        notes.insert( notes.end(), anacrusis.begin(), anacrusis.end());
        --count;
    }

    //
    // now copy count times
    //
    while (count--)
    {
        notes.insert( notes.end(), measure.begin(), measure.end());
    }

    notes.insert( notes.end(), last_fragment.begin(), last_fragment.end());

    new_measure();

}

void track_builder::push_note()
{
    if (current_note.seconds > .001)
    {
        notes.push_back( std::move( current_note));
    }
    current_note = {};
}

void track_builder::append_note( const string &note)
{
    push_note();
    current_note = { note, note_seconds};
}


void track_builder::pause()
{
    current_note.seconds += note_seconds;
}

void track_builder::end_nlet()
{
    note_seconds = tempo.top();
    tempo.pop();
}


void track_builder::end_track()
{
    cleanup();
}

track_builder::sound_pointer track_builder::notes_to_bar(
    const note_vector &notes)
{
    sound_vector sounds;
    note last_note{};
    for (const auto &note : notes)
    {
        if (note.name.empty())
        {
            last_note.seconds += note.seconds;
        }
        else
        {
            if (last_note.seconds > .001)
            {
                sounds.push_back( current_instrument->get_note( last_note.name, last_note.seconds));
            }
            last_note = note;
        }
    }

    if (last_note.seconds > .001)
    {
        sounds.push_back( current_instrument->get_note( last_note.name, last_note.seconds));
    }

    return join_sounds( sounds.begin(), sounds.end(), Concatenate);
}

void track_builder::end_bar()
{
    push_note();
    if (!notes.empty())
    {
        track.push_back( notes_to_bar(notes));
        notes.clear();
        last_measure_index = 0;
        anacrusis_index = 0;
    }
}


