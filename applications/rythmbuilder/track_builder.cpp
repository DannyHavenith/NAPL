#include "instrument.h"
#include "napl.h"
#include "track_builder.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

using namespace std;

namespace
{

    block_producer *Concatenate( block_producer *left, block_producer *right)
    {
        paste_mutator *paster = new paste_mutator();
        paster->LinkTo( left, 0);
        paster->LinkTo( right, 1);

        return paster;
    }

    block_producer *Add( block_producer *left, block_producer *right)
    {
            stream_header h;
            left->GetStreamHeader( h);
            sample_object_factory *factory_ptr = factory_factory::GetSampleFactory( h);
            binary_block_processor *adder = factory_ptr->GetAdder();
            delete factory_ptr;
            adder->LinkTo( left, right);

            return adder;
    }

    //
    // join all samples in a range into a single sequence.
    // Doesn't really need RAIterators, but is a lot slower otherwise.
    //
    template< class RAIterator, class Op>
    block_producer *join_sounds( RAIterator begin, const RAIterator &end, Op op)
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
    template<typename iterator_t>
        void spread_bars( iterator_t begin, iterator_t end)
    {
        if (begin == end) return;

        iterator_t current = begin;

        size_t count = std::distance( begin, end);

        stream_header h;
        (*begin)->GetStreamHeader( h);
        sample_object_factory *factory_ptr = factory_factory::GetSampleFactory( h);

        if (count > 1)
        {
            short step = 2 * (32767 / static_cast<short>(count-1));
            short pan_position = -32768;
            while (current != end)
            {
                block_producer *prod_ptr = * current;
                block_mutator *panner = factory_ptr->GetPan( pan_position);
                panner->LinkTo( prod_ptr);
                *current = panner;
                pan_position += step;
                ++current;
            }
            current = begin;
        }
    }


}

void track_builder::log( std::string_view message)
{
    logging_stream << message << std::endl;
}

track_builder::track_builder(
    instrument_factory &instruments_,
    const std::string &default_name,
    std::ostream &logging_stream)
    : note_seconds(.25),
    instruments( instruments_),
    last_measure_index(0),
    current_note_seconds( 0.0),
    track_name( default_name),
    logging_stream( logging_stream)
{


};

void track_builder::start_track( const string &rythm, const string &section, int bpm, const string &comment)
{
    log( "start track: " + rythm);
    note_seconds = 60.0/bpm;
    track_name = rythm.empty()?string("track"): rythm;
    section_name = section;
}

void track_builder::cleanup()
{
    last_measure_index = 0;

    push_note();
    if (!current_bar.empty())
    {
        track.push_back( current_bar);
        current_bar.clear();
    }

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
    typedef std::vector< block_producer *> temp_sound_vec;

    temp_sound_vec bars;
    BOOST_FOREACH( bar_vector &bar, track)
    {
        bars.push_back(
                join_sounds( bar.begin(), bar.end(), Concatenate)
            );
    }

    // spread over stereo space.
    spread_bars( bars.begin(), bars.end());

    // add all bars
    block_producer *result =
        join_sounds( bars.begin(), bars.end(), Add);

    std::string filename = track_name;
    if (!section_name.empty())
    {
        filename += "_" + section_name;
    }

    filename += ".wav";

    block_sink *file_writer = filefactory::GetBlockSink( filename.c_str());
    if (!file_writer)
    {
        throw std::runtime_error( "could not open file " + filename + " for writing, is it open in another application?");
    }

    file_writer->LinkTo( result);

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
    log( "start bar: " + bar_name);
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
    log( "start nlet: " + std::to_string(numerator) + "/" + std::to_string(denominator));
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
    log( "new measure");
    push_note();
    last_measure_index = (int)current_bar.size();
}


void track_builder::repeat( int count)
{

    if (count == 0) return;

    push_note();

    //
    // create a copy of the notes to repeat, because we're going to invalidate all iterators by
    // performing a copy into a vector.
    //
    bar_vector::iterator begin =
        (last_measure_index != current_bar.size()) ?
                (current_bar.begin() + last_measure_index)
            :   current_bar.begin();

    bar_vector fragment( begin, current_bar.end());

    //
    // now copy count-1 times
    //
    while (--count)
    {
        std::copy( fragment.begin(), fragment.end(),
            std::back_inserter( current_bar));
    }

    new_measure();

}

void track_builder::push_note()
{
    if (current_note_seconds > .001)
    {
        //
        // this will ask for a note with an empty name
        // (current_note_name == "")
        // if there is only silence to push
        // (for instance when the bar starts with silence)
        current_bar.push_back(
            current_instrument->get_note(
                current_note_name,
                current_note_seconds
                )
            );

    }

    current_note_name.clear();
    current_note_seconds = 0.0;
}

void track_builder::note( const string &note)
{
    push_note();
    current_note_name = note;
    current_note_seconds = note_seconds;
}


void track_builder::pause()
{
    current_note_seconds += note_seconds;
}



void track_builder::end_nlet()
{
    log( "end nlet");
    note_seconds = tempo.top();
    tempo.pop();
}


void track_builder::end_track()
{
    log( "end track: " + track_name);
    cleanup();
}


void track_builder::end_bar()
{
    log( "end bar");
    push_note();
    if (!current_bar.empty())
    {
        track.push_back( current_bar);
        current_bar.clear();
        last_measure_index = 0;
    }
}


