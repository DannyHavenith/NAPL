##          Copyright Danny Havenith 2006
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)

# simply include all subdirectories, assuming they all have an individual CMakeLists.txt file to
# build an app.

#file( GLOB subdirs *.)
#foreach( app ${subdirs})
#	add_subdirectory( ${app})
#endforeach( app)

add_subdirectory( rythmbuilder)
#add_subdirectory( syncy)

