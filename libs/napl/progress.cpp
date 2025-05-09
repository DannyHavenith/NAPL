//
#include "stdafx.h"

#include "convendi.h"
#include "samplety.h"
#include "samplebl.h"
#include "progress.h"


progress_monitor::progress_monitor( progress* progress_ptr)
{
	Register( progress_ptr);
}

progress_monitor::progress_monitor()
{
}

// does not actually mutate blocks but just monitors the progress in
// the chain.
void progress_monitor::Register( progress *progress_ptr)
{
	m_callbacks.push_back( progress_ptr);
}

/**
 * broadcast that we made a step in progress.
 * \param step percentage ( 0-1), completed
 */
void progress_monitor::broadcast_step( float step)
{
	callback_list_type::iterator i;
	for (i = m_callbacks.begin(); i != m_callbacks.end(); ++i)
	{
		(*i)->step( step);
	}
}


/**
 * set our current position. This will broadcast a progress
 * step to all listeners.
 * \param pos the position, should be between 0 and m_total.
 */
void progress_monitor::set_current_position( sampleno pos)
{
	if (m_total)
	{
		if (pos >= m_total) pos = m_total - 1;
		broadcast_step( float( pos)/ float( m_total));
	}
}

/**
 * block producer interface.
 * delegate the request to the producer, but also note our current position.
 * \param &consumer - the requesting consumer
 * \param start
 * \param num
 * \return
 */
sample_block progress_monitor::RequestBlock( sampleno start, unsigned long num)
{
	m_current_position = start;
	if (m_pProducer)
	{
		set_current_position( start);
		return m_pProducer->RequestBlock( start, num);
	}
	return {};
}

/**
 * block producer interface.
 * progress monitor will delegate this call to the real producer but eavesdrop
 * the number of frames.
 * \param &h
 */
void progress_monitor::GetStreamHeader( stream_header &h)
{
	m_pProducer->GetStreamHeader( h);
	m_total = h.numframes;
	m_framesize = h.frame_size();
}

/**
 * the text-based progress bar is initially hidden.
 * Only if real 'progress' is detected, the progress bar will
 * show.
 * \param &text
 * \param &strm
 * \param size
 * \return
 */
text_based_progress_bar::text_based_progress_bar( const std::string &text, std::ostream &strm, int size)
:m_stream( strm), m_size( size), m_text( text), m_hidden( true)
{
	// nop
}

/**
 *	this function is called by a progress monitor when there is progress
 * to report.
 * the text based bar will show  a row of '*'-characters to indicate progress.
 * \param progress
 */
void text_based_progress_bar::step( float progress)
{
	//
	// only start showing if the progress makes real steps, i.e.
	// when the progress is more than zero and is not immediately
	// at 100%
	//
	if (m_hidden && progress < 0.9 && progress > 0.001)
	{
		m_hidden = false;
		m_stream << m_text << std::endl;
	}

	//
	// show the actual progress bar
	//
	if (!m_hidden)
	{
		int filled = int((m_size-2) * progress + 0.4);
		int togo = m_size - 2 - filled;

		m_stream << char(13) << "[" << std::string( filled, '*') << std::string( togo, ' ') << ']';
	}
}

text_based_progress_bar::~text_based_progress_bar(void)
{
	if (!m_hidden) m_stream << std::endl;
}
