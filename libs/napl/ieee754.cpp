/* This code is a copy of the file type_conversion.c from the package
   AIFF_DSP by Ben Denckla.


*/

#include "stdafx.h"

#include "ieee754.h"

#define BIAS (0x4000 - 2)

//namespace ieee754
//{

/* i4() returns the number stored in 4-byte big-endian integer b.  The native long format need not be big-endian or limited to 4 bytes. */
unsigned long i4( UCHAR b[4] )
{
   return (((unsigned long) b[0]) << 24) |
          (((unsigned long) b[1]) << 16) |
          (((unsigned long) b[2]) <<  8) |
                            b[3];
}

unsigned int i2( UCHAR b[2] )
{
   return (((unsigned int) b[0]) << 8) | b[1];
}

/* c4() makes b into a 4-byte big-endian representation of n.  The native long format need not be big-endian or limited to 4 bytes. */
void c4( UCHAR b[4], unsigned long n )
{
   b[3] = (unsigned char)n;
   b[2] = (unsigned char)(n >>  8);
   b[1] = (unsigned char)(n >> 16);
   b[0] = (unsigned char)(n >> 24);
}

void c2( UCHAR b[2], unsigned int n )
{
   b[1] = n;
   b[0] = n >> 8;
}

/* convert_to_IEEE754() takes advantage of the fact that you don't have to worry about exponent range problems when dealing with sampling rates since they are very constrained in range. */
void convert_to_IEEE_754( double num, UCHAR bytes[10] )
{
   int sign, expon;
   unsigned long hiMant, loMant;
   double fMant, floMant, fhiMant;

   if (num == 0.0) expon = hiMant = loMant = 0;
   else
   {
      if (num < 0)
      {
         sign = 0x8000;
         num = -num;
      }
      else
         sign = 0;

      fMant = frexp(num, &expon);
	
      expon += BIAS;
      expon |= sign;
	    
      floMant = modf( ldexp(fMant,   32), &fhiMant );
      floMant =       ldexp(floMant, 32);

      hiMant = (unsigned long)fhiMant; 
      loMant = (unsigned long)floMant; /* fractional part will be discarded as per K&R p.197 */
   }

   c2( bytes,     expon  );
   c4( bytes + 2, hiMant );
   c4( bytes + 6, loMant );
}

double convert_fr_IEEE_754( UCHAR bytes[10] )
{
   double f;
   int expon, sign;
   unsigned long hiMant, loMant;

   sign = bytes[0] & 0x80; /* record sign */
   bytes[0] &= 0x7F;       /* then get rid of it in the original */
    
   expon  = i2( bytes     );
   hiMant = i4( bytes + 2 );
   loMant = i4( bytes + 6 );
            
   if (expon == 0 && hiMant == 0 && loMant == 0)
      f = 0.0;
   else
   {
      expon -= BIAS;
      f  = ldexp( (double) hiMant, expon-=32);
      f += ldexp( (double) loMant, expon-=32);
   }

   return sign ? -f : f;
}

//} // namespace ieee754

