struct napl_error: public std::runtime_error
{
	napl_error( const std::string &message)
		: runtime_error( "NAPL error: " + message)
	{
	};

};
