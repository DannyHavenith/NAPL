//
// this code is not mine. Read ieee754.cpp to read the comment
//

//namespace ieee754
//{

#define UCHAR unsigned char 
void          byte_reorder( long buflen );

unsigned long i4( UCHAR b[4] );
unsigned int  i2( UCHAR b[2] );
void          c4( UCHAR b[4], unsigned long n );
void          c2( UCHAR b[2], unsigned int  n );


extern void          convert_to_IEEE_754 ( double num, unsigned char bytes[10] );
extern double        convert_fr_IEEE_754 (             unsigned char bytes[10] );
//}

//using ieee754::convert_fr_IEEE_754;
//using ieee754::convert_to_IEEE_754;
