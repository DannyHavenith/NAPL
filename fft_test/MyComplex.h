// myComplex.h
//

template< class T> 
struct Complex
{
private:
	T m_real;
	T m_imag;
	typedef Complex<T> this_type;

public:

	__forceinline Complex( const T &r)
		: m_real( r), m_imag(0){};

	__forceinline Complex()
			: m_real(0), m_imag(0){};
	__forceinline Complex( T real, T imag)
		: m_real( real), m_imag( imag){};

	__forceinline Complex( const this_type &other) 
		: m_real( other.real()), m_imag( other.imag())
	{};

	__forceinline const T &real() const
	{
		return m_real;
	}

	__forceinline const T &imag() const
	{
		return m_imag;
	}

	__forceinline const T &real( const T r)
	{
		return m_real = r;
	}

	__forceinline const T &imag( const T i)
	{
		return m_imag = i;
	}

	__forceinline const this_type &operator*=( const T &rhs)
	{
		m_imag *= rhs;
		m_real *= rhs;

		return *this;
	}

	__forceinline const this_type &operator+=(const this_type &rhs)
	{
		real( real() + rhs.real());
		imag( imag() + rhs.imag());
		return *this;
	}

	__forceinline const this_type &operator-=( const this_type &rhs)
	{
		real( real() - rhs.real());
		imag( imag() - rhs.imag());
		return *this;
	}

	__forceinline this_type operator+( const this_type &rhs) const
	{
		return this_type( real()  + rhs.real(), imag() + rhs.imag());
	}


	__forceinline this_type operator-( const this_type &rhs) const
	{
		return this_type( real() - rhs.real(), imag() - rhs.imag());
	}

	__forceinline const this_type &operator/=( const T rhs)
	{
		m_imag /= rhs;
		m_real /= rhs;
		return *this;
	}
};

template<class T1, class T2> 
__forceinline Complex<T1>& operator*=( Complex<T1>& lhs,
								const Complex<T2>& rhs)
										
{
	T1 rhsre = (T1)rhs.real();
	T1 rhsim = (T1)rhs.imag();
	T1 temp = lhs.real() * rhsre - lhs.imag() * rhsim;

	lhs.imag(lhs.real() * rhsim + lhs.imag() * rhsre);
	lhs.real(temp);
	return (lhs); 
}

template< class T>
ostream &operator<<( ostream &os, Complex<T> c)
{
	return os << "(" << c.real() << "," << c.imag() << ")";
}