#if !defined( PROGRESS_H)
#define PROGRESS_H

struct progress
{
	virtual void step( float progress) = 0;
};

/**
 * \ingroup Napl
 *
 * a progress monitor does not actually mutate the sample stream. It is inserted into
 * a Napl graph and sends progress notifications to a class that adheres to the 'progress'
 * interface.
 *
 * \version 1.0
 * first version
 *
 * \date 12-21-2004
 *
 * \author Danny
 *
 * \todo
 *
 * \bug
 *
 */
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

/**
 * \ingroup Napl
 *
 * A default implementation of the progress interface.
 * This class shows progress as a text line consisting of
 * asterisk ('*') characters.
 *
 * \version 1.0
 *
 * \date 12-21-2004
 *
 * \author Danny
 *
 * \todo
 *
 * \bug
 *
 */
struct text_based_progress_bar : public progress
{
	text_based_progress_bar( const std::string &text, std::ostream &strm, int size);
	virtual void step( float progress);
private:
	std::ostream &m_stream;
	int m_size;
	std::string m_text;
	bool m_hidden;

public:
	~text_based_progress_bar(void);
};

#endif //PROGRESS_H
