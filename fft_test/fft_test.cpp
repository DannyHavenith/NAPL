// fft_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MyComplex.h"
//#include "FixedPoint.h"

// very handy...
const double pi = 3.1415926535;

template< typename T>
struct my_traits
{
	typedef long double accumulator_type;
};

struct my_traits< float>
{
	typedef float accumulator_type;
};

struct my_traits< short>
{
	typedef long accumulator_type;
};

struct my_traits< char>
{
	typedef short accumulator_type;
};

struct my_traits< long>
{
	typedef __int64 accumulator_type;
};

template< typename T, int size> 
struct FFT
{
	void make_swap_table();

	FFT()
		:m_pTable(0) 
	{
		make_table();
		make_swap_table();
	};

	~FFT()
	{
		delete m_pTable;
	}

	void make_table();
	void transform( const T *input, Complex<T> * output);

	void print_list( Complex<T> *pList);

	typedef my_traits<T>::accumulator_type accumulator_type;
	typedef Complex< accumulator_type> table_type;
//	typedef Complex<float> accumulator_type;
//	typedef Complex< float> table_type;
	table_type *m_pTable;
//	char m_pTableTouch[size];

	static const accumulator_type scale;
	typedef std::vector< std::pair< int, int> > swap_table;
	typedef swap_table::iterator swap_iterator;

	swap_table m_swap_table;
};

template< typename T, int size>
const FFT< T, size>::accumulator_type FFT< T, size>::scale =  numeric_limits< T>::max() + 1;
//template< typename T, int size>
//const FFT< T, size>::accumulator_type FFT< T, size>::scale =  1;



template< typename T, int size>
void FFT< T, size>::make_table()
{
	Complex<double> accumulator;
	

	m_pTable = new table_type[ size];
	for (int i = 0; i < size; ++i)
	{
		accumulator = Complex<double>( cos( -2 * pi * i / size), sin( -2 * pi * i / size));
		m_pTable[i] = table_type( scale * accumulator.real(), scale * accumulator.imag());
		cout << m_pTable[i] << ",";
	}
	cout << endl;
};

template< typename T, int size>
void FFT< T, size>::print_list( Complex<T> *pList)
{
	for ( int i = 0; i < size; ++i)
	{
		cout << *pList++;
	}
	cout << endl;
}

template< typename T, int size>
void FFT< T, size>::transform( const T *pInp, Complex<T> *pOutp)
{
	register table_type xt;

	int t;

	for (t = size - 1; t >= 0; --t)
	{
		pOutp[t] = pInp[t];
	}

	Complex<T> *x = pOutp;
	register Complex<T> xt1;

	int N = size;
	int L = N;
	int M = log2<size>::value;
	int step = 1;
	table_type *pTable;
	Complex<T> *px, *px2;

	for (int i = 1; i <= M; ++i)
	{
		for (int k = 0; k < L; k += N)
		{
			pTable = m_pTable;
			px = x + k;
			px2= x + k + N/2;
			for ( int n = k; n < k + N/2; ++n)
			{
				//xt1 = *px - *px2 
				xt.real( px->real() - px2->real());
				xt.imag( px->imag() - px2->imag());
				xt  *= *pTable;
				pTable += step;
				xt /= scale;
				//xt ;

				*px += *px2;


				//xt /= scale;
				px2->real( xt.real());
				px2->imag( xt.imag());

				++px;
				++px2;
			}
		}
		N /= 2;
		step *= 2;
	}

	for (swap_iterator s = m_swap_table.begin(); s != m_swap_table.end(); ++s)
	{
		swap(  pOutp[s->first], pOutp[s->second]);
	}
	
}

template < class T, int size>
void FFT< T, size>::make_swap_table()
{
	for (int i = 0; i < size; ++i)
	{

		int ii = i; 
		int k = size/2;
		int l = 1;
		int r  = 0;
		for (int j = 0; j < log2<size>::value; ++j)
		{
			if ( i & l) 
			{
				r |= k;
			}
			l <<= 1;
			k >>=1;
		}
		if (r > i)
		{
			m_swap_table.push_back( make_pair( r, i));
		}
	}
	cout << "size of swaptable = " << m_swap_table.size() << endl;
}

typedef short sample_type;

sample_type testdata[32] = { 1000, 1000, 1000, 1000, 0, 0, 0, 0};

const int buffer_size = power2< log2<sizeof(testdata)/sizeof(testdata[0])>::value >::value;
Complex<sample_type> result[ buffer_size] = {Complex<sample_type>( 1.1f, 2.2f)};

int main(int argc, char* argv[])
{

	FFT< short, buffer_size> transformer;

	
	for (long x = 0; x < buffer_size; ++x)
	{
		testdata[x] = 1000 * sin( 2 * pi * x / buffer_size);
		//testdata[x] = 111;
		cout << testdata[x] << ",";
	}
	cout << endl;

	transformer.transform( testdata, result);

	transformer.print_list( result);
	return 0;
}

