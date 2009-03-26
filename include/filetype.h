/***********************************************************************************
**
** filetype.h - utility classes for several file types
**
**
/***********************************************************************************/
#ifndef _FILE_TYPE_H_
#define _FILE_TYPE_H_

#include <stdio.h>
#include <boost/cstdint.hpp>
#include "architec.h"
#include "samplebl.h"


#define SH_ARCH_BIGENDIAN ARCHITECTURE_BIGENDIAN

/*
*** A file_block_producer reads some audio file and delivers the contents
*** (the bytes) in blocks
***
*/
class file_block_producer : public creative_block_producer
{
public:
	virtual void GetStreamHeader( stream_header &h)
	{
		h = m_stStreamHeader;
	}

        file_block_producer( FILE *fp, const stream_header &h, unsigned long offset) :
		m_pFile( fp), m_stStreamHeader( h)
	{
		m_nBytesPerSample = (short)(h.get_sample_size() / 8 * h.numchannels);
		m_nFileOffset = offset;
	}

	virtual ~file_block_producer()
	{
		fclose( m_pFile);
	}


protected:
	//
	// hot spots that fill in the real functionality of this particular class
	//
	virtual void Seek( sampleno start)
	{
		fseek( m_pFile, m_nFileOffset + start * m_nBytesPerSample, SEEK_SET);
	}

	virtual sampleno FillBlock( sample_block &b, sampleno count)
	{
		if (count * m_nBytesPerSample > b.buffer_size())
		{
			count = sampleno( b.buffer_size() / m_nBytesPerSample);
		}
		b.m_end = b.m_start + count * m_nBytesPerSample;


		sampleno nRead = static_cast<sampleno>( fread( b.buffer_begin(), m_nBytesPerSample, count, m_pFile));

		return nRead;
	}

private:
	FILE *m_pFile;
	stream_header m_stStreamHeader;
	unsigned short m_nBytesPerSample;
	unsigned long m_nFileOffset;

};

/*
*** A file_type is a general interface that defines file types (AIFF or WAV for example)
*** that can be used.
*/
class file_type
{
public:
	virtual block_producer *MakeBlockProducer( const char *filename) = 0;
	virtual block_sink *MakeBlockSink( const char *filename) = 0;
};


class arch_endian_converter
{
protected:
	inline static boost::uint16_t endian_convert( boost::uint16_t in)
	{
		return (boost::uint16_t)(in << 8) | (in >> 8);
	}
	inline static boost::uint32_t endian_convert( boost::uint32_t in)
	{
		return endian_convert((boost::uint16_t) (in >> 16)) | (endian_convert((boost::uint16_t) in) << 16);
	}
	inline static boost::int64_t endian_convert( boost::int64_t in)
	{
		return endian_convert((boost::uint32_t) (in >> 32)) | ((boost::uint64_t)endian_convert((boost::uint32_t) in) << 32);
	}
};


class endian_non_converter
{
protected:
	inline static boost::uint16_t endian_convert( boost::uint16_t in)
	{
		return in;
	}
	inline static boost::uint32_t endian_convert( boost::uint32_t in)
	{
		return in;
	}
};

typedef boost::uint32_t ul;

#if (LOCAL_ARCHITECTURE == ARCHITECTURE_LITTLEENDIAN)

#define IDword( a, b, c, d)  (((ul)a) | (((ul)b) << 8) | (((ul)c) << 16) | (((ul)d) << 24))

typedef endian_non_converter little_endian_converter;
typedef arch_endian_converter big_endian_converter;

#else

#define IDword( a, b, c, d)  (((ul)d) | (((ul)c) << 8) | (((ul)b) << 16) | (((ul)a) << 24))

typedef endian_non_converter big_endian_converter;
typedef arch_endian_converter little_endian_converter;

#endif

//
// The global function 'stream' is a function that makes
//
// Streamable is an interface definition of classes that can be written to a C-style
// file.
//
// The function 'Stream' (with capital 'S') is the function that makes the streamable
// write itself to a file.

// the function 'stream' (lowercase 's') is a utility function that writes an object
// (specified in the second parameter) to a file.
//
class streamable
{
public:
	struct direction { virtual bool isoutput() const = 0;};
	struct output: public direction { virtual bool isoutput() const { return true;}}; // direction type for output
	struct input: public direction  { virtual bool isoutput() const { return false;}}; // direction type for input

	virtual bool Stream( FILE *fp, const streamable::direction &d) = 0;
};

///////////////////////////////////////////////////////////////////////////
//
// big_endian_file
// this template is a definition of a big_endian file
// it defines how several types must be written to a file.
//
//

// the general case assumes there is a 'Stream()' method
// notice the capital 'S' in the 'Stream' method.
template <class T>
struct big_endian_file : private big_endian_converter
{
	inline static bool stream( FILE * file, T &m, const streamable::direction &d)
	{
		return m.Stream( file, d);
	}
};

template<> struct big_endian_file<boost::uint32_t> : private big_endian_converter
{
	// long (4 bytes) int input/output.
	static bool stream( FILE * file, boost::uint32_t &c, const streamable::direction &d)
	{
		if (d.isoutput())
		{
		    boost::uint32_t temp = endian_convert( c);
			return (4 == fwrite( &temp, 1, 4, file));
		}
		else
		{
		    boost::uint32_t temp;
			if (4 != fread( &temp, 1, 4, file)) return false;
			c = endian_convert( temp);
			return true;
		}
	}
};

template <> struct big_endian_file< long double>: private big_endian_converter
{
	// long double input/output.
	static bool stream( FILE * file, long double &c, const streamable::direction &d)
	{
		if (d.isoutput())
		{
			return (10 == fwrite( &c, 1, 10, file));
		}
		else
		{
			return ( 10 == fread( &c, 1, 10, file));
		}
	}

};

template <> struct big_endian_file< boost::uint8_t> : private big_endian_converter
{
	// character input/output
	static bool stream( FILE * file, boost::uint8_t c, const streamable::direction &d)
	{
		if (d.isoutput())
		{
			return EOF != putc( c, file);
		}
		else
		{
			c = static_cast<boost::uint8_t> (getc(file));
			return !feof( file);
		}
	}

};

template <> struct big_endian_file< boost::uint16_t> : private big_endian_converter
{
	static bool stream( FILE * file, boost::uint16_t &c, const streamable::direction &d)
	{
		if (d.isoutput())
		{
		    boost::uint32_t temp = endian_convert( c);
			return (2 == fwrite( &temp, 1, 2, file));
		}
		else
		{
		    boost::uint16_t temp;
			if (fread( &temp, 1, 2, file) != 2) return false;
			c = endian_convert( temp);
			return true;
		}
	}
};

///////////////////////////////////////////////////////////////////////////
//
// little_endian_file
// this template is a 'traits'-like definition of a little_endian file
// it defines how several types must be written to a file.
//
//

// the general case assumes there is a 'Stream()' method
// notice the capital 'S' in the 'Stream' method.
template <class T>
struct little_endian_file : private little_endian_converter
{
	inline static bool stream( FILE * file, T &m, const streamable::direction &d)
	{
		return m.Stream( file, d);
	}
};

template <> struct little_endian_file<boost::uint32_t> : private little_endian_converter
{
	// long (4 bytes) int input/output.
	static bool stream( FILE * file, boost::uint32_t &c, const streamable::direction &d)
	{
		if (d.isoutput())
		{
		    boost::uint32_t temp = endian_convert( c);
			return (4 == fwrite( &temp, 1, 4, file));
		}
		else
		{
		    boost::uint32_t temp;
			if (4 != fread( &temp, 1, 4, file)) return false;
			c = endian_convert( temp);
			return true;
		}
	}
};

template <> struct little_endian_file< long double>: private little_endian_converter
{
	// long double input/output.
	static bool stream( FILE * file, long double &c, const streamable::direction &d)
	{
		if (d.isoutput())
		{
			return (10 == fwrite( &c, 1, 10, file));
		}
		else
		{
			return ( 10 == fread( &c, 1, 10, file));
		}
	}

};

template <> struct little_endian_file< boost::uint8_t> : private little_endian_converter
{
	// character input/output
	static bool stream( FILE * file, boost::uint8_t c, const streamable::direction &d)
	{
		if (d.isoutput())
		{
			return EOF != putc( c, file);
		}
		else
		{
			c = static_cast<boost::uint8_t> (getc(file));
			return !feof( file);
		}
	}
};

template <> struct little_endian_file< boost::uint16_t> : private little_endian_converter
{
	static bool stream( FILE * file, boost::uint16_t &c, const streamable::direction &d)
	{
		if (d.isoutput())
		{
		    boost::uint16_t temp = endian_convert( c);
			return (2 == fwrite( &temp, 1, 2, file));
		}
		else
		{
		    boost::uint16_t temp;
			if (fread( &temp, 1, 2, file) != 2) return false;
			c = endian_convert( temp);
			return true;
		}
	}
};



//
// a ChunkID always gets written verbatim to the file, regardless of endian-
// nes.
//
class ChunkID : public streamable
{
 protected:
     boost::uint32_t m_ChunkID;
 public:
	virtual bool Stream( FILE *file, const streamable::direction &d)
	{
		if (d.isoutput())
		{
			return (4 == fwrite( &m_ChunkID, 1, 4, file));
		}
		else
		{
		    boost::uint32_t lID;
			if (4 != fread( &lID, 1, 4, file)) return false;
			while (lID != m_ChunkID)
			{
			    boost::uint32_t lLength;
				if (4 != fread( &lLength, 1, 4, file)) return false;
				fseek( file, lLength, SEEK_CUR);
				if (4 != fread( &lID, 1, 4, file)) return false;
			}
			return true;
//			return ( 4 == fread( &m_ChunkID, 1, 4, file));
		}
	}
	ChunkID(){};
	ChunkID(boost::uint32_t l): m_ChunkID(l){};
};

template <typename T>
bool be_stream( FILE *fp, T &o, const streamable::direction &d)
{
	return big_endian_file< T>::stream( fp, o, d);
}

template <typename T>
bool le_stream( FILE *fp, T &o, const streamable::direction &d)
{
	return little_endian_file< T>::stream( fp, o, d);
}

#endif
