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

void progress_monitor::broadcast_step( float step)
{
	callback_list_type::iterator i;
	for (i = m_callbacks.begin(); i != m_callbacks.end(); ++i)
	{
		(*i)->step( step);
	}
}

void progress_monitor::set_current_position( sampleno pos)
{
	if (m_total)
	{
		if (pos >= m_total) pos = m_total - 1;
		broadcast_step( float( pos)/ float( m_total));
	}
}

block_result progress_monitor::RequestBlock( block_consumer &consumer, sampleno start, unsigned long num)
{
	state_saver< block_consumer *> saver( m_pConsumer);

	m_pConsumer = &consumer;
	m_current_position = start;
	if (m_pProducer) 
	{
		set_current_position( start);
		return m_pProducer->RequestBlock( *this, start, num);
	}
	return BLOCK_ERROR;
}

void progress_monitor::GetStreamHeader( stream_header &h)
{
	m_pProducer->GetStreamHeader( h);
	m_total = h.numframes;
	m_framesize = h.frame_size();
}

void progress_monitor::Seek( sampleno start)
{
	m_pProducer->Seek( start);
}

void progress_monitor::ReceiveBlock( const sample_block &b)
{
	m_current_position += (b.m_end - b.m_start) / m_framesize;
	set_current_position( m_current_position);
	m_pConsumer->ReceiveBlock( b);
}



text_based_progress_bar::text_based_progress_bar( const std::string &text, std::ostream &strm, int size)
:m_stream( strm), m_size( size), m_text( text), m_hidden( true)
{
	// nop
}

void text_based_progress_bar::step( float progress)
{
	int filled = int((m_size-2) * progress + 0.4);
	int togo = m_size - 2 - filled;

	if (m_hidden && progress < 0.9 && progress > 0.001)
	{
		m_hidden = false;
		m_stream << m_text << std::endl;
	}

	if (!m_hidden)
	{
		m_stream << char(13) << "[" << std::string( filled, '*') << std::string( togo, ' ') << ']';
	}
}

text_based_progress_bar::~text_based_progress_bar(void)
{
	if (!m_hidden) m_stream << std::endl;
}
