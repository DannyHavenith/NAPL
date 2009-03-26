#if !defined( NAPL_ERROR_H)
#define NAPL_ERROR_H
#include <stdexcept>
struct napl_error: public std::runtime_error
{
	napl_error( const std::string &message)
		: std::runtime_error( "NAPL error: " + message)
	{
	};

};
#endif //NAPL_ERROR_H
