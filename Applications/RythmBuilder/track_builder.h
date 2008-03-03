#ifndef __TRACK_BUILDER_H__
#define __TRACK_BUILDER_H__

#include <string>
#include <vector>
#include <stack>
#include <boost/shared_ptr.hpp>
#include <boost/assign.hpp>
#include <boost/filesystem/path.hpp>


#include "instrument_factory.h"

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
    void repeat( int count);
    void note( const string &note);
    void pause();

    void end_nlet();
    void end_track();
    void end_bar();

    track_builder( const boost::filesystem::path &instrument_path);

    ~track_builder(void)
    {
    }

private:
    void cleanup();
    void emit_track();
    void push_note();
    typedef block_producer *sound_pointer;

private:
    typedef std::vector< sound_pointer > bar_vector;
    typedef std::vector< bar_vector> bars_vector;
    typedef std::stack< double> tempo_stack;

    bars_vector track;
    bar_vector current_bar;
    tempo_stack tempo;
    double note_seconds;
    double current_note_seconds;
    std::string current_note_name;
    std::string track_name;
    std::string section_name;
    int last_measure_index;
    instrument_factory instruments;
    boost::shared_ptr<instrument> current_instrument;

};

#endif //__TRACK_BUILDER_H__