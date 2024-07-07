#include "instrument.h"
#include "napl.h"
#include "samplebl.h"
#include "simpops.h"
#include "track_builder.h"

#include <boost/lexical_cast.hpp>

#include <iterator>
#include <numeric>
#include <string>

using namespace std;

namespace
{
    using block_producer_ptr = std::shared_ptr< block_producer>;
    using sound_pointer = std::shared_ptr<block_producer>;
    using sound_vector  = std::vector<sound_pointer>;


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

    /**
     * spread the sound in the iterator range over the stereo space (so that the
     * first one is on the utter left channel and the last on the utter right.
     */
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

    /**
     * Calculate the total duration of a range of notes in seconds.
     *
     */
    double total_duration(
        note_vector::const_iterator begin,
        note_vector::const_iterator end)
    {
        return std::accumulate( begin, end, 0.0, []( double sum, const note &n)
        {
            return sum + n.seconds;
        });
    }

    /**
     * Create a single sound source out of a vector of notes.
     *
     * This function collapses repeated pauses into single pauses and will also
     * append pauses to the end of the last sound before that pause.
     *
     * @param instrument instrument to translate note names to sounds
     * @param notes
     * @return sound_pointer
     */
    sound_pointer notes_to_sound(
        instrument &instrument,
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
                    sounds.push_back( instrument.get_note( last_note.name, last_note.seconds));
                }
                last_note = note;
            }
        }

        if (last_note.seconds > .001)
        {
            sounds.push_back( instrument.get_note( last_note.name, last_note.seconds));
        }

        return join_sounds( sounds.begin(), sounds.end(), Concatenate);
    }

    /**
     * convert a vector of measures with repeat counts into a long string of notes.
     *
     * This function will take care of anacrusis notes and repeats.
     *
     * @param measures measures with potential repeat counts and anacrusis notes.
     * @return note_vector Long string of notes
     */
    note_vector measures_to_notes(
        const std::vector<repeated_measure> &measures)
    {
        note_vector notes;
        for (const auto &measure : measures)
        {
            note_vector last_fragment;
            auto repeat_count = measure.repeat_count;
            const auto duration = total_duration( measure.notes.begin(), measure.notes.end());
            if (measure.anacrusis_index)
            {
                const auto anacrusis_duration = total_duration( measure.notes.begin() + measure.anacrusis_index, measure.notes.end());
                note_vector anacrusis{}; // start with a conditional pause until the anacrusis
                anacrusis.insert( anacrusis.end(), measure.notes.begin() + measure.anacrusis_index, measure.notes.end());
                last_fragment.assign( measure.notes.begin(), measure.notes.begin() + measure.anacrusis_index);
                last_fragment.push_back( { "", anacrusis_duration, true});

                // insert anacrusis
                notes.push_back( { "", duration - anacrusis_duration, true});
                notes.insert( notes.end(), measure.notes.begin() + measure.anacrusis_index, measure.notes.end());

                repeat_count--;
            }

            while (repeat_count--)
            {
                notes.insert( notes.end(), measure.notes.begin(), measure.notes.end());
            }

            notes.insert( notes.end(), last_fragment.begin(), last_fragment.end());
        }

        return notes;
    }

    void write_file( const std::string &filename, block_producer *p)
    {
        std::unique_ptr<block_sink> file_writer{ filefactory::GetBlockSink( filename.c_str())};

        file_writer->LinkTo( p);

        //
        // write to file...
        file_writer->Start();
    }

    void write_track( const track &track, instrument_factory &instruments)
    {
        sound_vector bars;
        for (const auto &bar : track.bars)
        {
            const auto instrument = instruments.get_instrument( bar.instrument_name);
            bars.push_back(
                notes_to_sound(
                    *instrument,
                    measures_to_notes( bar.measures)));
        }
        sound_vector stereo_bars( bars.size());

        // Watch out! the stereo sound sources don't own the sound sources in bars,
        // so as soon as those go out of scope, the stereo sound sources will be invalid.
        spread_bars( bars.begin(), bars.end(), stereo_bars.begin());
        const auto track_sounds =  join_sounds( stereo_bars.begin(), stereo_bars.end(), Add);

        auto filename = track.name;
        if (not track.section.empty())
        {
            filename += "_" + track.section;
        }
        filename += ".wav";

        write_file( filename, track_sounds.get());
    }

} // namespace

track_builder::track_builder(
    instrument_factory &instruments_,
    const std::string &default_name)
    : note_seconds(.25),
    instruments( instruments_)
{
};

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

void track_builder::mark_anacrusis()
{
    current_measure.anacrusis_index = current_measure.notes.size();
}

void track_builder::repeat( int count)
{

    if (count == 0) return;
    current_measure.repeat_count = count;
    new_measure();
}

void track_builder::append_note( const string &note)
{
    current_measure.notes.push_back( { note, note_seconds});
}

void track_builder::pause()
{
    current_measure.notes.push_back( { "", note_seconds});
}

void track_builder::end_nlet()
{
    note_seconds = tempo.top();
    tempo.pop();
}

void track_builder::start_track( const string &rythm, const string &section, int bpm, const string &comment)
{
    note_seconds = 60.0/bpm;
    current_track = { rythm.empty()?string("track"): rythm, section, comment, {}};
}

//
// start a new bar.
// precondition: current_bar.empty() == true
//
void track_builder::start_bar( const string &instrument_name, const string &bar_name)
{
    current_bar = { bar_name, instrument_name, {}};
}

void track_builder::new_measure()
{
    if (not current_measure.notes.empty())
    {
        current_bar.measures.push_back( { std::move( current_measure)});
        current_measure = {};
    }
}

void track_builder::end_bar()
{
    new_measure();

    if (not current_bar.measures.empty())
    {
        current_track.bars.push_back( std::move( current_bar));
        if (not current_bar.bar_name.empty())
        {
            named_bars[current_bar.bar_name] = current_bar;
        }

        current_bar = {};
    }
}

void track_builder::end_track()
{
    end_bar();

    if (not current_track.bars.empty())
    {
        write_track( current_track, instruments);
        current_track = {};
    }
}



