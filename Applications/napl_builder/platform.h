

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


	/**
	 * \brief expand wildcards in a command line argument list.
	 *
	 * Accept a classic argc, argv pair and expand any arguments that contain wildcards into 
	 * the filenames that match these arguments. Note that on some platforms this function
	 * will merely copy the arguments to the new list, since the shell will already have
	 * expanded wildcards.
	 *
	 * \param &names [in, out] argument list to which the arguments will be added
	 * \param argc [in] number of elements in argv
	 * \param argv[] array of pointers to the argument strings.
	 */
	static void expand_wildcards( name_list_t &names, int argc, _TCHAR * argv[]);

private:
	static name_list_t expand_argument_wildcards(std::string argument);
};
