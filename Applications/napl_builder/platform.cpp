#include "stdafx.h"
#include "platform.h"
#include <windows.h>

using std::string;

/**
 * Try to expand a command line argument to a list of file names.
 * if expansion failes, we assume that the argument was not meant to be a filename
 * and we return the original argument.
 *
 * \param argument the original argument that may or may not contain wildcards
 * \return either a list of files or the original argument
 */
platform::name_list_t platform::expand_argument_wildcards(std::string argument)
{

	name_list_t names;
	::WIN32_FIND_DATA FindFileData;
	::HANDLE hFind;

	string path;
	string::size_type index = argument.find_last_of( "\\/");
	if (index != string::npos)
	{
		path = argument.substr( 0, index + 1); 
	}


	
	hFind = ::FindFirstFile( argument.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		names.push_back( argument);
	}
	else
	{
		bool go_on = true;
		while (go_on)
		{
			if (FindFileData.cFileName != string( ".") &&
				FindFileData.cFileName != string( ".."))
			{
				names.push_back( path + FindFileData.cFileName);
			}
			go_on = ::FindNextFile( hFind, &FindFileData);
		}
		
		::FindClose( hFind);
	}

	return names;
}


