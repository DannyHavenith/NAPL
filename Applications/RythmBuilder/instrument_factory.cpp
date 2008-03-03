#include "StdAfx.h"
#include "instrument.h"
#include "instrument_factory.h"

instrument_factory::instrument_factory( const boost::filesystem::path &p)
    : instrument_path( p)
{
}


boost::shared_ptr< instrument> instrument_factory::get_instrument( const std::string &name)
{
    typedef boost::shared_ptr<instrument> instrument_ptr;
    if (!instruments[name])
    {
        instruments[name] = instrument_ptr( new instrument( instrument_path / name));
    }

    return instruments[name];
}

