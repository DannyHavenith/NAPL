

/**
 * \ingroup napl_builder
 * Simple abstraction of platform specific functions.
 *
 * \version 1.0
 *
 * \date 12-21-2004
 *
 * \author Danny
 *
 *
 */
class platform
{
public:
	typedef std::vector< std::string> name_list_t;


	static void expand_wildcards( name_list_t &names, int argc, _TCHAR * argv[]);

private:
	static name_list_t expand_argument_wildcards(std::string argument);
};
