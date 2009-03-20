template< class T, int precision>
struct fixed_point
{
private:
	typedef fixed_point<T> this_type;

public:
	T m_value;

	fixed_point()
		: m_value(0){};
	fixed_point( const T value)
		: m_value( value << precision) {};

	operator T()
	{
		return m_value >> precision;
	}

	const this_type &operator+=( const this_type &other)
	{
		m_value += other.m_value;
		return *this;
	}
	const this_type &operator-=( const this_type &other)
	{
		m_value -= other.m_value;
		return *this;
	}
	const this_type &operator*=( const this_type &other)
	{
		m_value *= other.m_value;
		return *this;
	}
	const this_type &operator/=( const this_type &other)
	{
		m_value /= other.m_value;
		return *this;
	}

	const this_type operator-( const this_type &other)
	{
		return this_type( *this)-=other;
	}
	const this_type operator+( const this_type &other)
	{
		return this_type( *this) += other;
	}
	const this_type operator*( const this_type &other)
	{
		return this_type( *this) *= other;
	}
	const this_type operator/( const this_type &other)
	{
		return this_type( /this) /= other;
	}
};

operator<<( ostream &os, fixed_point<T> f)
{
	os << (T)(f) << "." << (T)(f - ((T)f)));
};
