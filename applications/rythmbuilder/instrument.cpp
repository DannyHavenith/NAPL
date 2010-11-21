#include "stdafx.h"
#include <map>
#include <fstream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/xpressive/xpressive_static.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/foreach.hpp>

#include "napl.h"
#include "instrument.h"


namespace {
    std::string get_file_contents( const boost::filesystem::path &p)
    {
        boost::filesystem::ifstream instream( p);
        instream.unsetf(std::ios::skipws);
        return std::string(
            std::istreambuf_iterator<char>(instream.rdbuf()),
            std::istreambuf_iterator<char>());
    }
}

void instrument::add_wav( const boost::filesystem::path &p, const std::string &name)
{
    using boost::algorithm::to_lower_copy;

    if (boost::filesystem::is_regular( p))
    {
        block_producer_ptr producer(filefactory::GetBlockProducer( p.external_file_string().c_str()));
        if (producer)
        {
            notes[ to_lower_copy( name)] = producer;
        }
    }
}

block_producer *instrument::get_note( const std::string &name, double seconds)
{
    using boost::algorithm::to_lower_copy;
    std::string name_lower = to_lower_copy( name);

    unsigned int microseconds = static_cast<unsigned long>( seconds * 1000000);
    block_producer_ptr note( cache[microseconds][name_lower]);
    if (!note)
    {
        note = create_note( name_lower, seconds);
        cache[microseconds][name_lower] = note;
    }

    return note.get();
}

instrument::block_producer_ptr instrument::create_note( const std::string &name, double seconds)
{
    block_producer_ptr new_note;
    block_producer_ptr original = notes[name];

    if (original)
    {
        stream_header h;
        original->GetStreamHeader( h);
        unsigned long needed_frames = static_cast<unsigned long>( seconds * h.samplerate);
        if (needed_frames < h.numframes)
        {
            cut_mutator *cutter = new cut_mutator();
            cutter->LinkTo( original.get());
            cutter->SetCut( 0, needed_frames);
            new_note = block_producer_ptr( cutter);
        }
        else if (needed_frames > h.numframes)
        {
            double original_seconds = (double( h.numframes)/h.samplerate);
            block_mutator *delay = factory_factory::GetSampleFactory( h)->GetDelay( 0, seconds - original_seconds);
            delay->LinkTo( original.get());
            new_note = block_producer_ptr(  delay);
        }
        else
        {
            new_note = original;
        }
    }
    else
    {
        throw std::runtime_error( "no such note:" + name);
    }
    return new_note;
}

void instrument::add_silence()
{
    if (!notes.empty())
    {
        // get details like samplerate and framesize
        // of the first note and use those to create a silence with the
        // same attributes.
        stream_header h;
        notes.begin()->second->GetStreamHeader( h);
        notes[""] = block_producer_ptr(
            factory_factory::GetSampleFactory( h)->GetConstant( h));

    }
}

instrument::instrument(const boost::filesystem::path &p)
{
    using namespace boost::filesystem;
    using namespace boost::xpressive;
    using namespace boost::algorithm;
    using namespace std;

    sregex wavfile = (s1 = +_ ) >> (as_xpr(".wav")|".aiff") >> eos;
    sregex aliasfile = (s1 = +_) >> ".alias" >> eos;

    typedef std::map<string, path> alias_map_t;
    alias_map_t aliases;

    if (!is_directory( p))
    {
        throw std::runtime_error( "the directory " + p.external_file_string() + " does not exist");
    }

    directory_iterator end_it;
    for (directory_iterator i( p); i != end_it; ++i)
    {
        smatch what;
        string filename = i->leaf();
        if (regex_match( filename, what, wavfile))
        {
            add_wav( *i, what[1]);
        }
        else if (regex_match( filename, what, aliasfile))
        {
            aliases[ to_lower_copy(string(what[1]))] = *i;
        }
    }

    // add the entry "" to the notes table that contains a silence
    add_silence();

    // now that all wavs have been found, we can try to add the aliases.
    typedef alias_map_t::value_type alias_t;
    BOOST_FOREACH( const alias_t &alias, aliases)
    {
        const string real_name = to_lower_copy( trim_copy( get_file_contents( alias.second)));
        note_table_t::const_iterator real = notes.find( real_name);
        if (real != notes.end())
        {
            notes[to_lower_copy( alias.first)] = real->second;
        }
    }
}

instrument::~instrument()
{
}
