struct progress
{
	virtual void step( float progress) = 0;
};

// does not actually mutate blocks but just monitors the progress in 
// the chain.
struct progress_monitor : public block_mutator
{
	progress_monitor( progress* progress_ptr);
	progress_monitor();

	void Register( progress *progress_ptr);
	virtual block_result RequestBlock( block_consumer &consumer, sampleno start, unsigned long num);
	virtual void GetStreamHeader( stream_header &h);
	virtual void Seek( sampleno start);
	virtual void ReceiveBlock( const sample_block &b);

private:
	void broadcast_step( float step);
	void set_current_position( sampleno pos);
	
	typedef std::vector<progress *> callback_list_type;
	callback_list_type m_callbacks;
	sampleno m_total;
	sampleno m_framesize;
	sampleno m_current_position;
};

struct text_based_progress_bar : public progress
{
	text_based_progress_bar( std::ostream &strm, int size);
	virtual void step( float progress);
private:
	std::ostream &m_stream;
	int m_size;
public:
	~text_based_progress_bar(void);
};
