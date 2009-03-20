//
// Several processors
//
/*
template <typename receiver>
class processor
{
public:
	typedef receiver::sample sample;
	typedef receiver::accumulator accumulator;

protected:
	receiver m_receiver;
};

template <typename receiver>
class buffer_processor : public processor< receiver>
{

protected:
	sample *m_end;
	sample *m_begin;
	sample *m_current;
	size_t m_currentsize;
	size_t m_size;


protected:
	// the following two structs are used to make 'pushreceiver' choose
	// between calling one of the two member functions of its receiver
	struct PushSelector
	{
	};

	struct BufferSelector
	{
	};

	// pushreceiver has one templatized parameter 'function_selector'
	// the parameter itself is not used, but the type of the parameter
	// determines if this function will call 'push_buffer' or 'push'
	//
	// Under normal circumstances the second parameter will be optimized 
	// to the eternal bit bucket in the skies.
	inline void pushreceiver( sample s, PushSelector)
	{
			m_receiver.push( s);
	}

	inline void pushreceiver( sample s, BufferSelector)
	{
			m_receiver.push_buffer( s);
	}
	
	inline bool buffer_not_full()
	{
		return m_currentsize < m_size;
	}

	inline void advance()
	{
		++m_current;
		if (m_current == m_end) m_current = m_begin;
	}

public:
	buffer_processor( size_t size)
	{
		m_begin = new sample[size];
		m_current = m_begin;
		m_end = m_begin + size;
		m_size = size;
		m_currentsize = 0;
	}

};

template <typename receiver>
class midler : public buffer_processor< receiver>
{
private:
	accumulator m_total;
	// make room in the buffer by releasing the oldest sample.
	// which is coincidentally pointed to by 'm_current'.
	// functionselector is used to force the right kind of
	// 'pushreceiver' and will be optimized to bits by the compiler.
	template <class functionselector>
	inline void release_current( functionselector f)
	{
		pushreceiver( *m_current - m_total / m_currentsize, f);
		m_total -= *m_current;
	}

public:
	template <class functionselector>
	inline void process( sample s, functionselector f)
	{
		release_current( f);
		*m_current = s;
		advance();
		m_total += s;
	}

public:
	midler( size_t size): buffer_processor<receiver>( size)
	{
		m_total = 0;
	};


	inline void push_buffer( sample s)
	{
		BufferSelector selectbuffer;

		if (buffer_not_full())
		{
			*m_current = s;
			m_total += s;
			++m_currentsize;
			advance();
		}
		else
			// call a version of process that will call
			// 'push_buffer' of our receiver.
			process(s, selectbuffer);
	}

	inline void push( sample s)
	{
		PushSelector selectpush;
		// call a version of 'process' that will call
		// 'push' of our receiver.
		process( s, selectpush);
	}

	inline void finish()
	{
		PushSelector selectpush;
		sample *stop_here = m_current;
		if (buffer_not_full()) m_current = m_begin;
		do
		{
			release_current( selectpush);
			advance();
		} while (m_current != stop_here);

	}
};
*/