#include "instrument.h"
#include "instrument_factory.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

instrument_factory::instrument_factory( const boost::filesystem::path &p)
    : instrument_path( p)
{
    typedef std::shared_ptr<instrument> instrument_ptr;
    using namespace boost::filesystem;

    directory_iterator end_it;
    for (directory_iterator i( p); i != end_it; ++i)
    {
        if (is_directory( *i))
        {
            instruments[i->path().filename().string()] = instrument_ptr(
                new instrument( *i));

        }
    }
}

std::set< std::string> instrument_factory::get_note_names()
{
    typedef std::set<std::string> stringset;
    stringset names;
    BOOST_FOREACH( instrument_map::value_type &instr, instruments)
    {
        stringset instrument_names = instr.second->get_note_names();
        names.insert( instrument_names.begin(), instrument_names.end());
    }

    return names;
}

std::shared_ptr< instrument> instrument_factory::get_instrument( const std::string &name)
{
    typedef std::shared_ptr<instrument> instrument_ptr;
    if (!instruments[name])
    {
        instruments[name] = instrument_ptr( new instrument( instrument_path / name));
    }

    return instruments[name];
}

