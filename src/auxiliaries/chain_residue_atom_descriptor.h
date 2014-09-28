#ifndef AUXILIARIES_CHAIN_RESIDUE_ATOM_DESCRIPTOR_H_
#define AUXILIARIES_CHAIN_RESIDUE_ATOM_DESCRIPTOR_H_

#include <string>
#include <sstream>
#include <limits>
#include <stdexcept>
#include <cstdlib>
#include <vector>

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

	ChainResidueAtomDescriptor(const std::string& chainID) : serial(null_num()), chainID(chainID), resSeq(null_num())
	{
	}

	template<typename T>
	ChainResidueAtomDescriptor(const T& obj, const std::string& chainID) : serial(obj.serial), chainID(chainID), resSeq(obj.resSeq), resName(obj.resName), name(obj.name), altLoc(obj.altLoc), iCode(obj.iCode)
	{
	}

	static int null_num()
	{
		return std::numeric_limits<int>::min();
	}

	static const ChainResidueAtomDescriptor& solvent()
	{
		static const ChainResidueAtomDescriptor v("solvent");
		return v;
	}

	static std::string from_str(const std::string& input_str, ChainResidueAtomDescriptor& output_descriptor)
	{
		static const MarkerNaming mn;
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
		std::vector<std::string> markers;
		while(input.good())
		{
			std::string marker;
			input >> marker;
			if(!marker.empty())
			{
				if(marker==mn.chainID)
				{
					input >> v.chainID;
				}
				else if(marker==mn.resSeq)
				{
					input >> v.resSeq;
				}
				else if(marker==mn.iCode)
				{
					input >> v.iCode;
				}
				else if(marker==mn.serial)
				{
					input >> v.serial;
				}
				else if(marker==mn.altLoc)
				{
					input >> v.altLoc;
				}
				else if(marker==mn.resName)
				{
					input >> v.resName;
				}
				else if(marker==mn.name)
				{
					input >> v.name;
				}
				else
				{
					return (std::string("Unrecognized marker '")+marker+"' in descriptor string '"+input_str+"'.");
				}
				if(input.fail())
				{
					return (std::string("Unreadable marker '")+marker+"' value in descriptor string '"+input_str+"'.");
				}
				markers.push_back(marker);
			}
		}
		if(!(v.valid() && v.str(markers)==input_str))
		{
			return (std::string("Invalid descriptor string '")+input_str+"'.");
		}
		output_descriptor=v;
		return std::string();
	}

	static ChainResidueAtomDescriptor from_str(const std::string& input_str)
	{
		ChainResidueAtomDescriptor v;
		const std::string error_msg=from_str(input_str, v);
		if(!error_msg.empty())
		{
			throw std::runtime_error(error_msg);
		}
		return v;
	}

	static bool match_with_member_selection_string(const ChainResidueAtomDescriptor& crad, const std::string& input_str)
	{
		static const MarkerNaming mn;
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
		if(!input.fail() && mn.valid(marker))
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
					if(marker==mn.resSeq || marker==mn.serial)
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
								if(marker==mn.resSeq) { matched=matched || (crad.resSeq>=value.first && crad.resSeq<=value.second); }
								else if(marker==mn.serial) { matched=matched || (crad.serial>=value.first && crad.serial<=value.second); }
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
								if(marker==mn.resSeq) { matched=matched || (crad.resSeq==value); }
								else if(marker==mn.serial) { matched=matched || (crad.serial==value); }
							}
						}
					}
					else
					{
						control_output << token;
						if(marker==mn.chainID) { matched=matched || (crad.chainID==token); }
						else if(marker==mn.iCode) { matched=matched || (crad.iCode==token); }
						else if(marker==mn.altLoc) { matched=matched || (crad.altLoc==token); }
						else if(marker==mn.resName) { matched=matched || (crad.resName==token); }
						else if(marker==mn.name) { matched=matched || (crad.name==token); }
					}
				}
			}
			control_output << vend;
		}
		const std::string control_output_str=control_output.str();
		if(control_output_str.empty() || control_output_str!=input_str)
		{
			throw std::runtime_error(std::string("Invalid selection string '")+input_str+"'.");
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

	ChainResidueAtomDescriptor without_numbering() const
	{
		if((*this)==solvent())
		{
			return (*this);
		}
		else
		{
			ChainResidueAtomDescriptor v;
			v.resName=resName;
			v.name=name;
			return v;
		}
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
		static const MarkerNaming mn;
		static const std::string markers_array[7]={mn.chainID, mn.resSeq, mn.iCode, mn.serial, mn.altLoc, mn.resName, mn.name};
		static const std::vector<std::string> markers(markers_array, markers_array+7);
		return str(markers);
	}

private:
	struct MarkerNaming
	{
		std::string serial;
		std::string chainID;
		std::string resSeq;
		std::string resName;
		std::string name;
		std::string altLoc;
		std::string iCode;

		MarkerNaming() :
			serial("a"),
			chainID("c"),
			resSeq("r"),
			resName("rn"),
			name("an"),
			altLoc("l"),
			iCode("i")
		{
		}

		bool valid(const std::string& marker_name) const
		{
			return (marker_name==serial
					|| marker_name==chainID
					|| marker_name==resSeq
					|| marker_name==resName
					|| marker_name==name
					|| marker_name==altLoc
					|| marker_name==iCode);
		}
	};

	static const char vbegin='[';
	static const char vend=']';

	std::string str(const std::vector<std::string>& markers) const
	{
		const MarkerNaming mn;
		std::ostringstream output;
		for(std::size_t i=0;i<markers.size();i++)
		{
			const std::string& m=markers[i];
			if(m==mn.chainID && !chainID.empty())
			{
				output << mn.chainID << vbegin << chainID << vend;
			}
			else if(m==mn.resSeq && resSeq!=null_num())
			{
				output << mn.resSeq << vbegin << resSeq << vend;
			}
			else if(m==mn.iCode && !iCode.empty())
			{
				output << mn.iCode << vbegin << iCode << vend;
			}
			else if(m==mn.serial && serial!=null_num())
			{
				output << mn.serial << vbegin << serial << vend;
			}
			else if(m==mn.altLoc && !altLoc.empty())
			{
				output << mn.altLoc << vbegin << altLoc << vend;
			}
			else if(m==mn.resName && !resName.empty())
			{
				output << mn.resName << vbegin << resName << vend;
			}
			else if(m==mn.name && !name.empty())
			{
				output << mn.name << vbegin << name << vend;
			}
		}
		return output.str();
	}
};

}

#endif /* AUXILIARIES_CHAIN_RESIDUE_ATOM_DESCRIPTOR_H_ */
