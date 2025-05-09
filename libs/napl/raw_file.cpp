#include "samplebl.h"
#include "raw_file.h"

#include <fstream>

using namespace std;

class raw_file_sink : public block_sink
{
public:
	raw_file_sink( const char *filename)
		:file_(filename, std::ios::binary)
	{}

    void Start() override
    {
        FetchAll();
    }

	void ReceiveBlock( const sample_block &b) override
	{
		file_.write( reinterpret_cast< const char *>(b.m_start), b.m_end - b.m_start);
	};

private:
	ofstream file_;
};


block_sink *raw_file::MakeBlockSink( const char *filename)
{
	return new raw_file_sink( filename);
}

block_producer *raw_file::MakeBlockProducer( const char *filename)
{
	return 0;
}
