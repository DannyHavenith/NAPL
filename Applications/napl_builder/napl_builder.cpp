// napl_builder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "sample_analyzer.h"
#include <boost/spirit/error_handling/exceptions.hpp>
#include "napl_facade.h"
using namespace boost::javascript;
using namespace boost::clipp;

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;

void error( string message)
{
	cerr << message << endl;
	exit( -1);
}

void print( string x)
{
	std::cout << x << endl;
}

int napl_system( string command)
{
	return system( command.c_str());
}

void init_napl_context( context *c)
{
	napl_facade::init( c);
	block_producer_wrapper::init( c);

	// add the convenience function 'writeln' 
	function(c,"writeln",print);
	function( c, "system", napl_system);
}

	
// run the script that is contained in 'script'.
// pass the arguments in argc and argv to the script
int run_script( string script, int argc, _TCHAR * argv[])
{
	//Create a javascript parser
    javascript_parser parser;
    context* c=parser.get_context();
	init_napl_context( c);	


	//
	// create a function that will call the main()-function with the 
	// right arguments.
	//
	string arguments = string("\nfunction __napl_main(){ \nvar args = [ \"") + argv[0] + "\"";
	for (int count = 1; count < argc; ++count)
	{
		arguments += (string(", \"") + argv[count] + "\"");
	}
	arguments += "];\nreturn main( args);};";

	// add this function at the end of the script.
	script += arguments;


	try
	{
		parser.parse( script);
	}
	catch ( boost::spirit::parser_error_base &)
	{
		error( "a parse error occurred");
	}
	catch( std::runtime_error &e)
	{
		error( string("a runtime error occurred: ") + e.what());
	}

	int output = -255;

	// find the main function 
	valueP main_func = parser.global()["__napl_main"];
	if (main_func)
	{

		try
		{
			valueP result = main_func();

			//Extract answer 
			//output = unwrap<int>(result)();
		}
		catch( bad_type_conversion &)
		{
			error( "cannot find the main function");
		}
		catch( std::runtime_error &e)
		{
			error( string("a runtime error occurred: ") + e.what());
		}
	}
	return output;
}

int _tmain(int argc, _TCHAR* argv[])
{
	int retval = -1;
	if (argc < 2)
	{
		error( string("usage: ") + argv[0] + " <scriptfile> [script args...]");
	}

	std::ifstream instream( argv[1]);    

	if (instream)
	{

		std::string script(
			std::istreambuf_iterator<char>(instream.rdbuf()),
			std::istreambuf_iterator<char>());
		retval = run_script( script, argc - 1, argv + 1);
	}
	else
	{
		error( string( "could not open script file: ") + argv[1]);
	} 


	return retval;
}
