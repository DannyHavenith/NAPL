#include "StdAfx.h"
#include "napl_facade.h"
using std::string;
using namespace boost::clipp;

void file_writer::init( boost::clipp::context * c)
{
	class_<file_writer> cls( "FileWriter", c);
	cls.constructor( arguments< std::string>());
	cls.function( "writeln", writeln);
	cls.function( "write", write);
	cls.function( "close", close);
}

void file_writer::write(std::string str)
{
	m_stream << str;
}

void file_writer::writeln(std::string str)
{
	write( str);
	m_stream << std::endl;
}

void file_writer::close(void)
{
	m_stream.close();
}

file_writer::file_writer(std::string filename)
:m_stream( filename.c_str())
{
}

file_writer * file_writer::create_one(std::string filename)
{
	return new file_writer( filename);
}

