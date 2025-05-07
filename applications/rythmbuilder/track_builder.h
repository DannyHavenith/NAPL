#ifndef __TRACK_BUILDER_H__
#define __TRACK_BUILDER_H__

#include "instrument_factory.h"

#include <boost/assign.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

class block_producer;
class instrument;

struct note
{
    std::string name;   ///< note name, empty for pause
    double      seconds;///< duration of the note in seconds
    bool optional = false; /// for pauses added before anacrusis and after final bar
};

using note_vector = std::vector<note>;
struct measure
{
    note_vector notes;
    std::size_t anacrusis_index = 0;
};

struct repeated_measure : public measure
{
    int     repeat_count = 1;
};

struct bar
{
    std::string bar_name;
    std::string instrument_name;
    std::vector< repeated_measure> measures;
};
using bars_vector = std::vector< bar>;

struct track
{
    std::string name;
    std::string section;
    std::string comment;
    bars_vector bars;
};

inline std::ostream &print(std::ostream &os, const note &n)
{
    os << n.name << "(" << n.seconds << ")";
    return os;
}

inline std::ostream &print(std::ostream &os, const note_vector &nv)
{
    for (const auto &n : nv)
    {
        print(os, n) << " ";
    }
    return os;
}

inline std::ostream &print(std::ostream &os, const measure &m)
{
    os << "MEASURE:\n";
    os << "ANACRUSIS: " << m.anacrusis_index << "\n";
    print(os, m.notes);
    os << "\n";
    return os;
}

inline std::ostream &print(std::ostream &os, const repeated_measure &rm)
{
    os << "REPEATED MEASURE:\n";
    os << "REPEAT: " << rm.repeat_count << "\n";
    print(os, static_cast<const measure &>(rm));
    return os;
}

inline std::ostream &print(std::ostream &os, const bar &b)
{
    os << "BAR: " << b.bar_name << "\n";
    os << "INSTRUMENT: " << b.instrument_name << "\n";
    for (const auto &rm : b.measures)
    {
        print(os, rm);
    }
    return os;
}

inline std::ostream &print(std::ostream &os, const bars_vector &bv)
{
    for (const auto &b : bv)
    {
        print(os, b);
    }
    return os;
}

inline std::ostream &print(std::ostream &os, const track &t)
{
    os << "TRACK: " << t.name << "\n";
    os << "SECTION: " << t.section << "\n";
    os << "COMMENT: " << t.comment << "\n";
    print(os, t.bars);
    return os;
}

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
    void start_bar( const string &instrument_name, const string &bar_name);
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
        const std::string &default_name = "track");

private:


    static double total_duration( note_vector::const_iterator begin, note_vector::const_iterator end);

private:
    using tempo_stack = std::stack< double>;

    std::map< std::string, bar> named_bars;
    track current_track;
    bar current_bar;
    repeated_measure current_measure;
    tempo_stack tempo;
    double note_seconds;
    instrument_factory &instruments;
    std::shared_ptr<instrument> current_instrument;
};

#endif //__TRACK_BUILDER_H__
