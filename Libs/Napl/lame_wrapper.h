#pragma once

// classic PIMPL 
struct lame_wrapper_impl;

class lame_stream
{
private:
	lame_wrapper_impl *pimpl;
	unsigned long stream_handle;
public:
	lame_stream( lame_wrapper_impl *impl, unsigned long handle)
		: pimpl( impl), stream_handle( handle)
	{
		//nop
	}
};

/**
 * \ingroup Napl
 * \brief wrapper around the lame mp3 encoder DLL
 *
 * \version 1.0
 * first version
 *
 * \date 02-21-2005
 *
 * \author Danny
 *
 * This class wraps the lame mp3 encoder dll. It implements a singleton
 * instance that will try to load the dll. If the dll cannot be loaded, the
 * instance pointer will be null.
 */
class lame_wrapper
{
public:

	// public, so that we can be auto_ptr'd
	~lame_wrapper(void);

	// singleton pattern
	static lame_wrapper * get_instance(void);

private:
	// classic PIMPL implementation
	lame_wrapper(void);

	lame_wrapper_impl *pimpl;
public:
	bool is_ok(void);
private:
	lame_wrapper_impl *get_impl(void);
public:
	lame_stream * init_stream(unsigned long input_samplerate,
										unsigned short output_bitrate,
										unsigned short channels);
};
