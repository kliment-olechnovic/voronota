#ifndef AUXILIARIES_CHAIN_RESIDUE_ATOM_COMMENT_H_
#define AUXILIARIES_CHAIN_RESIDUE_ATOM_COMMENT_H_

#include <string>
#include <sstream>
#include <limits>
#include <stdexcept>
#include <cstdlib>

namespace auxiliaries
{

class ChainResidueAtomDescriptor
{
public:
	int serial;
	std::string chainID;
	int resSeq;
	std::string resName;
	std::string name;
	std::string altLoc;
	std::string iCode;

	ChainResidueAtomDescriptor() : serial(null_num()), resSeq(null_num())
	{
	}

	static int null_num()
	{
		return std::numeric_limits<int>::min();
	}

	static ChainResidueAtomDescriptor solvent()
	{
		ChainResidueAtomDescriptor v;
		v.chainID="solvent";
		return v;
	}

	static ChainResidueAtomDescriptor from_str(const std::string& input_str)
	{
		ChainResidueAtomDescriptor v;
		std::string refined_input_str=input_str;
		for(std::size_t i=0;i<refined_input_str.size();i++)
		{
			char& s=refined_input_str[i];
			if(s==vend || s==vbegin)
			{
				s=' ';
			}
		}
		std::istringstream input(refined_input_str);
		while(input.good())
		{
			std::string marker;
			input >> marker;
			if(!marker.empty())
			{
				if(marker=="c")
				{
					input >> v.chainID;
				}
				else if(marker=="r")
				{
					input >> v.resSeq;
				}
				else if(marker=="i")
				{
					input >> v.iCode;
				}
				else if(marker=="a")
				{
					input >> v.serial;
				}
				else if(marker=="l")
				{
					input >> v.altLoc;
				}
				else if(marker=="rn")
				{
					input >> v.resName;
				}
				else if(marker=="an")
				{
					input >> v.name;
				}
				else
				{
					throw std::runtime_error(std::string("Unrecognized marker '")+marker+"' in comment string '"+input_str+"'.");
				}
				if(input.fail())
				{
					throw std::runtime_error(std::string("Unreadable marker '")+marker+"' value in comment string '"+input_str+"'.");
				}
			}
		}
		if(!(v.valid() && v.str()==input_str))
		{
			throw std::runtime_error(std::string("Invalid comment string '")+input_str+"'.");
		}
		return v;
	}

	static bool match_with_member_descriptor(const ChainResidueAtomDescriptor& comment, const std::string& input_str)
	{
		const char vsep=',';
		const char vinterval=':';
		std::ostringstream control_output;
		bool matched=false;
		std::string refined_input_str=input_str;
		for(std::size_t i=0;i<refined_input_str.size();i++)
		{
			char& s=refined_input_str[i];
			if(s==vend || s==vbegin || s==vsep)
			{
				s=' ';
			}
		}
		std::istringstream input(refined_input_str);
		std::string marker;
		input >> marker;
		if(!input.fail() && (marker=="c" || marker=="r" || marker=="i" || marker=="a" || marker=="l" || marker=="rn" || marker=="an"))
		{
			control_output << marker << vbegin;
			bool need_sep=false;
			while(input.good())
			{
				std::string token;
				input >> token;
				if(!input.fail() && !token.empty())
				{
					if(need_sep) { control_output << vsep; } else { need_sep=true; }
					if(marker=="r" || marker=="a")
					{
						std::size_t pinterval=token.find(vinterval);
						if(pinterval!=std::string::npos)
						{
							token[pinterval]=' ';
							std::istringstream token_input(token);
							std::pair<int, int> value(0, 0);
							token_input >> value.first >> value.second;
							if(!token_input.fail() && value.first<=value.second)
							{
								control_output << value.first << vinterval << value.second;
								if(marker=="r") { matched=matched || (comment.resSeq>=value.first && comment.resSeq<=value.second); }
								else if(marker=="a") { matched=matched || (comment.serial>=value.first && comment.serial<=value.second); }
							}
						}
						else
						{
							std::istringstream token_input(token);
							int value=0;
							token_input >> value;
							if(!token_input.fail())
							{
								control_output << value;
								if(marker=="r") { matched=matched || (comment.resSeq==value); }
								else if(marker=="a") { matched=matched || (comment.serial==value); }
							}
						}
					}
					else
					{
						control_output << token;
						if(marker=="c") { matched=matched || (comment.chainID==token); }
						else if(marker=="i") { matched=matched || (comment.iCode==token); }
						else if(marker=="l") { matched=matched || (comment.altLoc==token); }
						else if(marker=="rn") { matched=matched || (comment.resName==token); }
						else if(marker=="an") { matched=matched || (comment.name==token); }
					}
				}
			}
			control_output << vend;
		}
		const std::string control_output_str=control_output.str();
		if(control_output_str.empty() || control_output_str!=input_str)
		{
			throw std::runtime_error(std::string("Invalid match descriptor '")+input_str+"'.");
		}
		return matched;
	}

	static bool match_with_sequence_separation_interval(const ChainResidueAtomDescriptor& a, const ChainResidueAtomDescriptor& b, const int min_sep, const int max_sep, const bool uncheckable_result)
	{
		if(!(min_sep==null_num() && max_sep==null_num()) && a.resSeq!=null_num() && b.resSeq!=null_num() && a.chainID==b.chainID)
		{
			int sep=abs(a.resSeq-b.resSeq);
			if(sep==0 && !(a.iCode.empty() && b.iCode.empty()) && a.iCode!=b.iCode)
			{
				if(!a.iCode.empty() && !b.iCode.empty())
				{
					sep=abs(static_cast<int>(a.iCode[0])-static_cast<int>(b.iCode[1]));
				}
				else
				{
					sep=1;
				}
			}
			return ((min_sep==null_num() || sep>=min_sep) && (max_sep==null_num() || sep<=max_sep));
		}
		return uncheckable_result;
	}

	bool valid() const
	{
		return ((!chainID.empty() || resSeq!=null_num() || !resName.empty() || serial!=null_num() || !name.empty()) &&
				!(resSeq==null_num() && !iCode.empty()) &&
				!(serial==null_num() && !altLoc.empty()));
	}

	ChainResidueAtomDescriptor without_atom() const
	{
		ChainResidueAtomDescriptor v=(*this);
		v.serial=null_num();
		v.altLoc.clear();
		v.name.clear();
		return v;
	}

	bool contains(const ChainResidueAtomDescriptor& v) const
	{
		return (valid() && v.valid() &&
				(v.serial==null_num() || v.serial==serial) &&
				(v.chainID.empty() || v.chainID==chainID) &&
				(v.resSeq==null_num() || v.resSeq==resSeq) &&
				(v.resName.empty() || v.resName==resName) &&
				(v.name.empty() || v.name==name) &&
				(v.altLoc.empty() || v.altLoc==altLoc) &&
				(v.iCode.empty() || v.iCode==iCode));
	}

	bool operator==(const ChainResidueAtomDescriptor& v) const
	{
		return (serial==v.serial && resSeq==v.resSeq && chainID==v.chainID && iCode==v.iCode && altLoc==v.altLoc && resName==v.resName && name==v.name);
	}

	bool operator<(const ChainResidueAtomDescriptor& v) const
	{
		if(chainID<v.chainID) { return true; }
		else if(chainID==v.chainID)
		{
			if(resSeq<v.resSeq) { return true; }
			else if(resSeq==v.resSeq)
			{
				if(iCode<v.iCode) { return true; }
				else if(iCode==v.iCode)
				{
					if(serial<v.serial) { return true; }
					else if(serial==v.serial)
					{
						if(altLoc<v.altLoc) { return true; }
						else if(altLoc==v.altLoc)
						{
							if(resName<v.resName) { return true; }
							else if(resName==v.resName)
							{
								return (name<v.name);
							}
						}
					}
				}
			}
		}
		return false;
	}

	std::string str() const
	{
		std::ostringstream output;
		if(!chainID.empty())
		{
			output << "c" << vbegin << chainID << vend;
		}
		if(resSeq!=null_num())
		{
			output << "r" << vbegin << resSeq << vend;
			if(!iCode.empty())
			{
				output << "i" << vbegin << iCode << vend;
			}
		}
		if(serial!=null_num())
		{
			output << "a" << vbegin << serial << vend;
			if(!altLoc.empty())
			{
				output << "l" << vbegin << altLoc << vend;
			}
		}
		if(!resName.empty())
		{
			output << "rn" << vbegin << resName << vend;
		}
		if(!name.empty())
		{
			output << "an" << vbegin << name << vend;
		}
		return output.str();
	}

private:
	static const char vbegin='[';
	static const char vend=']';
};

}

#endif /* AUXILIARIES_CHAIN_RESIDUE_ATOM_COMMENT_H_ */
