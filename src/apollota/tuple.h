#ifndef APOLLOTA_TUPLE_H_
#define APOLLOTA_TUPLE_H_

#include <vector>
#include <algorithm>

namespace voronota
{

namespace apollota
{

template<unsigned int N>
class Tuple
{
public:
	struct HashFunctor
	{
		std::size_t operator()(const Tuple& t) const
		{
			return t.hash_value();
		}
	};

	Tuple()
	{
		for(unsigned int i=0;i<N;i++)
		{
			v_[i]=0;
		}
	}

	explicit Tuple(const std::vector<std::size_t>& values)
	{
		for(unsigned int i=0;i<N && i<values.size();i++)
		{
			v_[i]=values[i];
		}
		sort();
	}

	Tuple(const Tuple<N-1>& shorter, const std::size_t tail)
	{
		for(unsigned int i=0;i<(N-1);i++)
		{
			v_[i]=shorter.get(i);
		}
		v_[N-1]=tail;
		sort();
	}

	Tuple(const std::size_t a, const std::size_t b)
	{
		if(0<N) v_[0]=a;
		if(1<N) v_[1]=b;
		sort();
	}

	Tuple(const std::size_t a, const std::size_t b, const std::size_t c)
	{
		if(0<N) v_[0]=a;
		if(1<N) v_[1]=b;
		if(2<N) v_[2]=c;
		sort();
	}

	Tuple(const std::size_t a, const std::size_t b, const std::size_t c, const std::size_t d)
	{
		if(0<N) v_[0]=a;
		if(1<N) v_[1]=b;
		if(2<N) v_[2]=c;
		if(3<N) v_[3]=d;
		sort();
	}

	unsigned int size() const
	{
		return N;
	}

	std::size_t get(unsigned int i) const
	{
		return v_[i];
	}

	bool contains(std::size_t x) const
	{
		for(unsigned int i=0;i<N;i++)
		{
			if(get(i)==x)
			{
				return true;
			}
		}
		return false;
	}

	unsigned int count(std::size_t x) const
	{
		unsigned int num=0;
		for(unsigned int i=0;i<N;i++)
		{
			if(get(i)==x)
			{
				num++;
			}
		}
		return num;
	}

	Tuple<N-1> exclude(unsigned int i) const
	{
		std::vector<std::size_t> values;
		values.reserve(N-1);
		for(unsigned int j=0;j<N;j++)
		{
			if(j!=i)
			{
				values.push_back(get(j));
			}
		}
		return Tuple<N-1>(values);
	}

	int number_of_subtuple(const Tuple<N-1>& subtuple) const
	{
		for(unsigned int i=0;i<N;i++)
		{
			if((!subtuple.contains(get(i))) && (subtuple==exclude(i)))
			{
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	bool has_repetetions() const
	{
		for(unsigned int i=0;i+1<N;i++)
		{
			if(v_[i]==v_[i+1])
			{
				return true;
			}
		}
		return false;
	}

	std::pair<std::size_t, std::size_t> get_min_max() const
	{
		std::pair<std::size_t, std::size_t> result(v_[0], v_[0]);
		for(unsigned int i=1;i<N;i++)
		{
			result.first=std::min(result.first, v_[i]);
			result.second=std::max(result.second, v_[i]);
		}
		return result;
	}

	std::size_t hash_value() const
	{
		std::size_t h=0;
		for(unsigned int i=0;i<N;i++)
		{
			h += get(i);
			h += (h << 10);
			h ^= (h >> 6);
		}
		h += (h << 3);
		h ^= (h >> 11);
		h += (h << 15);
		return h;
	}

	bool operator==(const Tuple& t) const
	{
		for(unsigned int i=0;i<N;i++)
		{
			if(get(i)!=t.get(i))
			{
				return false;
			}
		}
		return true;
	}

	bool operator<(const Tuple& t) const
	{
		for(unsigned int i=0;i<N;i++)
		{
			if(get(i)<t.get(i))
			{
				return true;
			}
			else if(get(i)>t.get(i))
			{
				return false;
			}
		}
		return false;
	}

private:
	void sort()
	{
		std::sort(v_, v_+N);
	}

	std::size_t v_[N];
};

typedef Tuple<2> Pair;
typedef Tuple<3> Triple;
typedef Tuple<4> Quadruple;

}

}

#endif /* APOLLOTA_TUPLE_H_ */
