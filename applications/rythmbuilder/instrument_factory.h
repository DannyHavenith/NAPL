#ifndef __INSTRUMENT_FACTORY_H__
#define __INSTRUMENT_FACTORY_H__

#include <string>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
class instrument;


class instrument_factory
{
public:

    instrument_factory( const boost::filesystem::path &p);
    std::shared_ptr< instrument> get_instrument( const std::string &instrument_name);
    std::set< std::string> get_note_names();

private:
    typedef std::map< std::string, std::shared_ptr< instrument> > instrument_map;
    instrument_map instruments;
    boost::filesystem::path instrument_path;
};

#endif //__INSTRUMENT_FACTORY_H__
