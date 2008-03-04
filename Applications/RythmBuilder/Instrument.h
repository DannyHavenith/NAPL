#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

#include "map_keys.hpp" // my own map_keys function

class block_producer;

class instrument
{
public:
    instrument( const boost::filesystem::path &path);
    ~instrument();

    block_producer *get_note( const std::string &name, double seconds);

    template< typename ContainerType>
    void get_note_names( ContainerType &c)
    {
        c = map_keys( notes);
    }

private:
    typedef boost::shared_ptr< block_producer> block_producer_ptr;
    void add_wav( const boost::filesystem::path &path, const std::string &name);
    block_producer_ptr create_note( const std::string &name, double seconds);
    void add_silence();

private:
    typedef std::map< std::string, block_producer_ptr> note_table_t;
    typedef std::map< unsigned long, note_table_t> note_cache_t;

    //
    // original note samples
    note_table_t notes;

    //
    // cache with notes of a certain size
    note_cache_t cache;
};

#endif //__INSTRUMENT_H__