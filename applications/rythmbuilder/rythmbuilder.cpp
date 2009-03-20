// RythmBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BOOST_SPIRIT_DEBUG
#include <exception>
#include <boost/spirit/qi.hpp>
#include <boost/assign.hpp>
#include <string>

#include "RythmParser.h"

using namespace std;

boost::filesystem::path find_instrument_path( const boost::filesystem::path &exe_path)
{
    // implementation for windows only:
    return exe_path.branch_path() / "instruments";
}

int main(int argc, char* argv[])
{
    


    if (argc != 2) return -1;

    std::ifstream instream( argv[1]);

    if (!instream.is_open()) {
        std::cerr << "could not open input file: " << argv[1]
                    << std::endl;
        return -2;
    }

    instream.unsetf(std::ios::skipws);
    std::string storage(
        std::istreambuf_iterator<char>(instream.rdbuf()),
        std::istreambuf_iterator<char>());

    std::cout << storage << endl;


    bool r = false;

    try
    {
        r = ParseRythm(  find_instrument_path( argv[0]), storage);
    }
    catch (std::exception &e)
    {
        std::cerr << "Something went wrong: " << e.what() << std::endl;
    }

    if (!r)
    {
        std::cerr << "Could not parse rythm" << std::endl;
    }

	return 0;
}

