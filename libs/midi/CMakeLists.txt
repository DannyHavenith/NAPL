##          Copyright Danny Havenith 2006 - 2009.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)

# simply include all subdirectories, assuming they all have an individual CMakeLists.txt file to 
# build a library.

#file( GLOB subdirs *.)
#foreach( lib ${subdirs})
#	add_subdirectory( ${lib})
#endforeach( lib) 

add_subdirectory( midi)
