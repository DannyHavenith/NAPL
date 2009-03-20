#include <fstream>
#include <iostream>

using namespace std;

main( int argc, char *argv[])
{
	if (argc == 2)
	{
		ofstream to;
		to.open( argv[1], ios_base::binary);
		
		for (int i = 0 ; i < 256; ++i)
		{
			to.put( i);
		}

		to.close();

		ifstream from( argv[1], ios_base::binary);

		if (!from) 
		{
			cerr << "kan de input niet openen" << endl;
			return -1;
		}

		char ch;

		while ( from.get( ch))
		{
			cout << (int)ch << "\t";
		}
	}
	return 0;
}