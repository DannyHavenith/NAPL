

class platform
{
public:
	typedef std::vector< std::string> name_list_t;



	static name_list_t expand_argument_wildcards(std::string argument);
};
