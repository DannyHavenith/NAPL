// RythmBuilder.cpp : Defines the entry point for the console application.
//

#define BOOST_SPIRIT_DEBUG
#include "rythmparser.h"

#include <boost/assign.hpp>
#include <boost/filesystem/path.hpp>

#include <exception>
#include <fstream>
#include <iostream>
#include <string>


using namespace std;

#if defined(_WIN32)
boost::filesystem::path find_instrument_path( const boost::filesystem::path &exe_path)
{
    // implementation for windows only:
    return exe_path.branch_path() / "instruments";
}
#endif

#if defined(linux)
boost::filesystem::path find_instrument_path( const boost::filesystem::path &exe_path)
{
    // implementation for linux:
    const auto home = getenv("HOME");
    if (home)
    {
        return boost::filesystem::path(home) / ".local/share/rythmbuilder/instruments";
    }
    else
    {
        return "/usr/local/share/rythmbuilder/instruments";
    }
}
#endif

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

    boost::filesystem::path input_path( argv[1]);
    const std::string filename = input_path.stem().string();

    bool r = false;

    try
    {
        r = ParseRythm(  find_instrument_path( argv[0]), storage, filename);
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

