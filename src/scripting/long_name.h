#ifndef SCRIPTING_LONG_NAME_H_
#define SCRIPTING_LONG_NAME_H_

#include <string>
#include <vector>
#include <set>

namespace voronota
{

namespace scripting
{

class LongName
{
public:
	template<class Collection>
	static std::set<std::size_t> match(const Collection& objects, const bool from_all, const std::set<std::size_t>& from_ids, const LongName& longname)
	{
		std::set<std::size_t> ids;
		for(std::size_t i=0;i<objects.size();i++)
		{
			if(objects[i].match(longname) && (from_all || from_ids.count(i)>0))
			{
				ids.insert(ids.end(), i);
			}
		}
		return ids;
	}

	template<class Collection>
	static std::set<std::size_t> match(const Collection& objects, const LongName& longname)
	{
		return match(objects, true, std::set<std::size_t>(), longname);
	}

	LongName()
	{
	}

	explicit LongName(const std::vector<std::string>& subnames) : subnames_(subnames)
	{
	}

	explicit LongName(const std::string& subname) : subnames_(std::vector<std::string>(1, subname))
	{
	}

	LongName(const std::vector<std::string>& subnames, const std::string& subname) : subnames_(subnames)
	{
		subnames_.push_back(subname);
	}

	LongName(const std::vector<std::string>& subnames, const std::string& subname1, const std::string& subname2) : subnames_(subnames)
	{
		subnames_.push_back(subname1);
		subnames_.push_back(subname2);
	}

	bool valid() const
	{
		if(subnames_.empty())
		{
			return false;
		}
		for(std::size_t i=0;i<subnames_.size();i++)
		{
			if(subnames_[i].empty())
			{
				return false;
			}
		}
		return true;
	}

	const std::vector<std::string>& subnames() const
	{
		return subnames_;
	}

	void set_subnames(const std::vector<std::string>& subnames)
	{
		subnames_=subnames;
	}

	bool match(const LongName& longname) const
	{
		return match(longname.subnames());
	}

	friend std::ostream& operator<<(std::ostream& output, const LongName& ln)
	{
		output << ln.subnames().size();
		for(std::size_t i=0;i<ln.subnames().size();i++)
		{
			output << " " << ln.subnames()[i];
		}
		output << "\n";
		return output;
	}

	friend std::istream& operator>>(std::istream& input, LongName& ln)
	{
		std::vector<std::string> subnames;
		int n=0;
		input >> n;
		if(n>0)
		{
			subnames.resize(n);
			for(std::size_t i=0;i<subnames.size();i++)
			{
				input >> subnames[i];
			}
		}
		ln=LongName(subnames);
		return input;
	}

private:
	bool match(const std::vector<std::string>& ref_subnames) const
	{
		if(ref_subnames.size()>subnames_.size())
		{
			return false;
		}
		for(std::size_t i=0;i<ref_subnames.size();i++)
		{
			if(subnames_[i]!=ref_subnames[i])
			{
				return false;
			}
		}
		return true;
	}

	std::vector<std::string> subnames_;
};

}

}

#endif /* SCRIPTING_LONG_NAME_H_ */
