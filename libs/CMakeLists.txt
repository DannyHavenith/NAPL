##          Copyright Danny Havenith 2010.
## Distributed under the Boost Software License, Version 1.0.
##    (See accompanying file LICENSE_1_0.txt or copy at
##          http://www.boost.org/LICENSE_1_0.txt)

# simply include all subdirectories, assuming they all have an individual CMakeLists.txt file to 
# build a library.

# find all first-level child directories that have a CMakeLists.txt-file
file( GLOB subdirs */CMakeLists.txt)

foreach( lib ${subdirs})
	string( REPLACE "/CMakeLists.txt" "" lib_dir ${lib})
	message( STATUS "adding " ${lib_dir})
	add_subdirectory( ${lib_dir})
endforeach( lib) 

