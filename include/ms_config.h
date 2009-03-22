////////////////////////////////////////////////////////////////////////////////
//
// config.h - define our local machine
//

// define here whether the target system is little- or big endian
// #define LOCAL_ARCHITECTURE ARCHITECTURE_BIGENDIAN // for big endian systems
//
#define LOCAL_ARCHITECTURE ARCHITECTURE_LITTLEENDIAN

// napl needs a 64 bit integer type for some calculations.
// fill in this type at the left side of this typedef
//
// example:
// typedef long long int64bit; // for GNU c++
//
typedef long long int64bit; // for microsoft C++

// napl uses 'strcmpi' to compare strings, some systems have
// another case-insensitive case compare. If your target system
// does, uncomment the #define and specify your string compare function
// (the example uses the GNU strcasecmp)
//
//#define strcmpi strcasecmp

#define __forceinline FORCE_INLINE
