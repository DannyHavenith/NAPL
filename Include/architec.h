//
// architecture.h - several constants that describe a machine-architecture or a
// layout of sample memory.
//

#ifndef _ARCHITECTURE_H_
#define _ARCHITECTURE_H_

// is the data big- or little endian
#define SH_ARCH_ENDIAN 1
#define ARCHITECTURE_BIGENDIAN 1
#define ARCHITECTURE_LITTLEENDIAN 0

// do blocks contain samples or fourier-transformed data
#define SH_ARCH_DOMAIN 6
#define ARCHITECTURE_FAST_TRANSFORM 4 // which means that the order of samples is changed
#define ARCHITECTURE_FREQUENCY_DOMAIN 2
#define ARCHITECTURE_TIME_DOMAIN 0

////////////////////////////////////////////////////////////////////////////////
//
// include local information
//
#include "config.h"

#endif