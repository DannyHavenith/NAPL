template < int size> 
struct log2
{
	enum {value = log2<size/2>::value + 1};
};

struct log2<1> 
{
	enum {value = 0};
};

struct log2<0>
{
	enum {value = 0};
};

template< int size>
struct power2
{
	enum {value = power2< size -1>::value * 2};
};

struct power2<0>
{
	enum { value = 1 };
};

