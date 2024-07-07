add_executable(
	rythmbuilder

	instrument.cpp
	instrument_factory.cpp
	rythmbuilder.cpp
	rythmparser.cpp
	track_builder.cpp
	rythmparser.h
	track_builder.h
	instrument_factory.h
	instrument.h
	)

TARGET_LINK_LIBRARIES(
	rythmbuilder
	PRIVATE
	    project_options
	    napl ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})

