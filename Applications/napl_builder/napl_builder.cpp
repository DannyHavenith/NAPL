// napl_builder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include "sample_analyzer.h"
#include <boost/spirit/error_handling/exceptions.hpp>
#include "napl_facade.h"
#include "platform.h"
using namespace boost::javascript;
using namespace boost::clipp;

using std::string;
using std::cerr;
using std::cout;
using std::endl;
using std::vector;

bool load_script( const std::string filename, std::string &target);

void error( string message)
{
	cerr << message << endl;
	exit( -1);
}

void print( string x)
{
	std::cout << x << endl;
}

/**
 * Run a command on the command line.
 * \param command [in] the command to be executed
 * \return whatever the command returned.
 */
int napl_system( string command)
{
	return system( command.c_str());
}

/**
 * Add Napl objects to the JavaScript environment
 * \param *c  context of the parser
 */
void init_napl_context( context *c)
{
	napl_facade::init( c);
	block_producer_wrapper::init( c);

	// add the convenience functions 'writeln' and 'system'
	function(c,"writeln",print);
	function( c, "system", napl_system);
}


/**
 * escape any special characters by prefixing them with '\'.
 * \param raw_str [in] the raw, unescaped string
 * \return the escaped string
 */
std::string escape_chars( std::string raw_str)
{
	string::size_type p = 0;
	while (string::npos != (p = raw_str.find_first_of( "\\\"", p)))
	{
		raw_str = raw_str.insert( p,"\\");
		p += 2;
	}

	return raw_str;
}

/**
 * run the script that is contained in 'script'.
 * pass the arguments in argc and argv to the script
 * \param script the script string
 * \param argc count of the commandline arguments
 * \param argv[] commandline arguments
 * \return the integer returnvalue of the script
 * \todo the output of the script
 */
int run_script( string script, int argc, _TCHAR * argv[])
{
	//Create a javascript parser
    javascript_parser parser;
    context* c=parser.get_context();
	init_napl_context( c);	

	platform::name_list_t cl_arguments;

	// the first argument is assumed to be the script name. do not try to expand
	cl_arguments.push_back( argv[0]);
	++argv;
	--argc;

	//
	// expand all other arguments
	//
	platform::expand_wildcards( cl_arguments, argc, argv);

	//
	// create a function that will call the main()-function with the 
	// right arguments.
	//
	string argument_stub_function = 
		string("\nfunction __napl_main(){ \nvar args = [ \"") + escape_chars( cl_arguments[0]) + "\"";
	platform::name_list_t::const_iterator i;
	for (i = cl_arguments.begin(), ++i; i != cl_arguments.end(); ++i)
	{
		argument_stub_function += (", \"" + escape_chars( *i) + "\"");
	}
	argument_stub_function += "];\nreturn main( args);};";

	// add this function at the end of the script.
	script += argument_stub_function;


	// parse the script
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

	//
	// run the script
	//
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


/**
 * parse #include-statements that may appear at the beginning of the file and replace them with the 
 * actual contents of the include file. 
 * This function will recursively expand any includes that occurr in included files as well.
 * \param &target the string to scan for includes.
 * \return true if successfull
 */
bool parse_includes( std::string &target)
{
	using namespace boost::spirit;

	//
	// store a list (vector) of all filenames to include
	//
	std::vector< std::string> includes;
	typedef std::vector< std::string>::const_iterator iterator_t;

	// define our 'grammar'
	rule<> comment = comment_p("//");
	rule<> include = '#' >> *blank_p  >> "include" >> *blank_p >>  confix_p('"', (*c_escape_ch_p)[push_back_a(includes)], '"');
	rule<> file_header = *((+space_p) | comment | include);

	parse_info<> info = parse( target.c_str(), file_header);

	// the vector 'includes' is now supposed to have an ordered list of 
	// files to include (it may be empty). Load the include files and
	// add them to the string 'header'
	std::string header;
	for ( iterator_t i = includes.begin(); i != includes.end(); ++i)
	{
		std::string include_script;
		if (!load_script( *i, include_script)) return false;
		header += include_script;
		header += "\n";
	}

	//
	// the string must now consist of the included text
	// and anything not parsed by the parser.
	//
	target = header + info.stop;

	return true;
}

/**
 * load a script file into the target string
 * this function will also recursively check for '#include' statements
 * and add the included files at the appropriate locations
 * \param filename (in) name of the script file to load
 * \param &target (out) string that receives the fully loaded script
 * \return 
 */
bool load_script( const std::string filename, std::string &target)
{
	bool retval = true;

	std::ifstream instream( filename.c_str());    
	if (instream)
	{
		
			
		target = std::string(
			std::istreambuf_iterator<char>(instream.rdbuf()),
			std::istreambuf_iterator<char>());

		retval = parse_includes( target);
	}
	else
	{
		error( string( "could not open script file: ") + filename);
		retval = false;
	}

	return retval;

}

int _tmain(int argc, _TCHAR* argv[])
{
	int retval = -1;
	if (argc < 2)
	{
		error( string("usage: ") + argv[0] + " <scriptfile> [script args...]");
	}


	std::string script;
	if (load_script( argv[1], script))
	{
		retval = run_script( script, argc - 1, argv + 1);
	}
	else
	{
		retval = -1;
	}

	return retval;
}

