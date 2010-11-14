add_executable( 
	rythmbuilder

	instrument.cpp
	instrument_factory.cpp
	rythmbuilder.cpp
#	rythmbuilder_old.cpp
	rythmparser.cpp
	track_builder.cpp
	stdafx.cpp
	rythmparser.h
	stdafx.h
	track_builder.h
	instrument_factory.h
	instrument.h
	note_table.h
	)

TARGET_LINK_LIBRARIES( rythmbuilder napl ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})

