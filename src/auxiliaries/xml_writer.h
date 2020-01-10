#ifndef AUXILIARIES_XML_WRITER_H_
#define AUXILIARIES_XML_WRITER_H_

#include <sstream>
#include <string>
#include <vector>
#include <map>

namespace voronota
{

namespace auxiliaries
{

class XMLWriter
{
public:
	explicit XMLWriter(const std::string& type) : type_(type)
	{
	}

	template<typename T>
	XMLWriter& set(const std::string& name, const T& value, const std::string& units="")
	{
		std::ostringstream output;
		output << value << units;
		parameters_[name]=output.str();
		return (*this);
	}

	XMLWriter& set(const std::string& contents)
	{
		contents_=contents;
		return (*this);
	}

	XMLWriter& add_child(const XMLWriter& child)
	{
		children_.push_back(child);
		return (*this);
	}

	void write(std::ostream& output, const std::size_t tabs=0) const
	{
		output << std::string(tabs, ' ') << "<" << type_;
		for(std::map<std::string, std::string>::const_iterator it=parameters_.begin();it!=parameters_.end();++it)
		{
			output << " " << it->first << "=\"" << it->second << "\"";
		}
		if(contents_.empty() && children_.empty())
		{
			output << "/>\n";
		}
		else
		{
			output << ">\n";
			if(!contents_.empty())
			{
				output << std::string(tabs+2, ' ') << contents_ << "\n";
			}
			for(std::size_t i=0;i<children_.size();i++)
			{
				children_[i].write(output, tabs+2);
			}
			output << "</" << type_ << ">\n";
		}
	}

private:
	std::string type_;
	std::map<std::string, std::string> parameters_;
	std::string contents_;
	std::vector<XMLWriter> children_;
};

}

}

#endif /* AUXILIARIES_XML_WRITER_H_ */
