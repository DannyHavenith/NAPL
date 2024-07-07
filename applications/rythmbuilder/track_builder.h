#ifndef __TRACK_BUILDER_H__
#define __TRACK_BUILDER_H__

#include "instrument_factory.h"

#include <boost/assign.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>
#include <memory>
#include <numeric>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

class block_producer;
class instrument;

class track_builder
{
public:
    struct no_instrument_exception : public std::runtime_error
    {
        no_instrument_exception( const std::string &what)
            : std::runtime_error( what)
        {
        };
    };

    typedef std::string string;

    void start_track( const string &rythm, const string &section, int bpm, const string &comment);
    void start_bar( const string &bar_name, const string &instrument_name);
    void start_nlet( int numerator, int denominator);

    void new_measure();
    void mark_anacrusis();
    void repeat( int count);
    void append_note( const string &note);
    void pause();

    void end_nlet();
    void end_track();
    void end_bar();


    track_builder(
        instrument_factory &instruments_,
        const std::string &default_name = "track",
        std::ostream &logging_stream = std::cout);

private:
    struct note
    {
        std::string name;
        double      seconds;
    };

    using note_vector = std::vector< note >;
    using sound_pointer = std::shared_ptr< block_producer>;

    void cleanup();
    void emit_track();
    void push_note();
    sound_pointer notes_to_bar(
        const note_vector &notes);

    static double total_duration( note_vector::const_iterator begin, note_vector::const_iterator end);

private:
    using sound_vector  = std::vector< sound_pointer >;
    using bars_vector = sound_vector;
    using tempo_stack = std::stack< double>;

    bars_vector track;
    note_vector notes;
    tempo_stack tempo;
    double note_seconds;
    note current_note;
    std::string track_name;
    std::string section_name;
    std::size_t last_measure_index;
    std::size_t anacrusis_index;
    instrument_factory &instruments;
    std::shared_ptr<instrument> current_instrument;
    std::ostream &logging_stream;

};

#endif //__TRACK_BUILDER_H__
