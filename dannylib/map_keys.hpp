#ifndef __MAP_KEYS_HPP__
#define __MAP_KEYS_HPP__
#include <boost/assign.hpp>
#include <boost/iterator/transform_iterator.hpp>
//
// class that transforms a map (or multi-map) to a container with only the keys
// of that map
//
template< typename map_type>
struct iterator_defs
{
    typedef typename map_type::value_type map_value_type;
    typedef typename map_type::key_type map_key_type;
    typedef map_key_type value_type;
    //
    // I'm not using bind here, because I don't want any complex typeof-business while defining
    // the transform iterators.
    //
    struct return_first
    {
        typedef value_type result_type;

        value_type operator()( map_value_type const &p) const
        {
            return p.first;
        }
    };

    typedef boost::transform_iterator< return_first, typename map_type::const_iterator> iterator_type;
//    typedef boost::transform_iterator< return_first, typename map_type::iterator> iterator;
};


template< typename map_type>
struct map_key_reference : 
    public boost::assign_detail::converter< map_key_reference<map_type>, typename iterator_defs<map_type>::iterator_type >


{

    typedef iterator_defs< map_type> defs;

    map_key_reference( const map_type &m)
        : the_map( m)
    {
    };

    typename defs::iterator_type begin() const
    {
        return typename defs::iterator_type( the_map.begin(), typename defs::return_first());
    }

    typename defs::iterator_type end() const
    {
        return typename defs::iterator_type( the_map.end(), typename defs::return_first());
    }

    template< class Container >
    operator Container() const
    {
        return this-> BOOST_NESTED_TEMPLATE convert_to_container<Container>();
    }

    const map_type &the_map;
};

template < typename map_type>
map_key_reference< map_type> map_keys( const map_type &map)
{
    return map_key_reference< map_type>( map);
}

#endif //__MAP_KEYS_HPP__
