#ifndef AUXILIARIES_CHAIN_RESIDUE_ATOM_DESCRIPTOR_H_
#define AUXILIARIES_CHAIN_RESIDUE_ATOM_DESCRIPTOR_H_

#include <string>
#include <sstream>
#include <limits>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>
#include <map>
#include <cmath>

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

	explicit ChainResidueAtomDescriptor(const std::string& chainID) : serial(null_num()), chainID(chainID), resSeq(null_num())
	{
	}

	ChainResidueAtomDescriptor(
			const int serial,
			const std::string& chainID,
			const int resSeq,
			const std::string& resName,
			const std::string& name,
			const std::string& altLoc,
			const std::string& iCode) :
				serial(serial),
				chainID(chainID),
				resSeq(resSeq),
				resName(resName),
				name(name),
				altLoc(altLoc),
				iCode(iCode)
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

	static const ChainResidueAtomDescriptor& any()
	{
		static const ChainResidueAtomDescriptor v("any");
		return v;
	}

	static std::string from_str(const std::string& input_str, ChainResidueAtomDescriptor& output_descriptor)
	{
		static const MarkerNaming mn;
		ChainResidueAtomDescriptor v;
		std::string refined_input_str=input_str;
		{
			char last_bracket='?';
			for(std::size_t i=0;i<refined_input_str.size();i++)
			{
				char& s=refined_input_str[i];
				if(s==vend || s==vbegin)
				{
					if(s==last_bracket)
					{
						return (std::string("Invalid bracketing in descriptor string '")+input_str+"'.");
					}
					last_bracket=s;
					s=' ';
				}
			}
			if(last_bracket!=vend)
			{
				return (std::string("Incomplete bracketing in descriptor string '")+input_str+"'.");
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
		else if(max_sep!=null_num() && a.chainID!=b.chainID)
		{
			return false;
		}
		return uncheckable_result;
	}

	bool valid() const
	{
		return ((!chainID.empty() || resSeq!=null_num() || !resName.empty() || serial!=null_num() || !name.empty()) &&
				!(resSeq==null_num() && !iCode.empty()));
	}

	ChainResidueAtomDescriptor without_some_info(const bool no_serial, const bool no_name, const bool no_resSeq, const bool no_resName) const
	{
		ChainResidueAtomDescriptor v=(*this);
		if(no_serial)
		{
			v.serial=null_num();
		}
		if(no_name)
		{
			v.name.clear();
		}
		if(no_resSeq)
		{
			v.resSeq=null_num();
		}
		if(no_resName)
		{
			v.resName.clear();
		}
		return v;
	}

	ChainResidueAtomDescriptor without_atom() const
	{
		return without_some_info(true, true, false, false);
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

	bool operator!=(const ChainResidueAtomDescriptor& v) const
	{
		return (!((*this)==v));
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
			resName("R"),
			name("A"),
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

	static const char vbegin='<';
	static const char vend='>';

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

inline std::ostream& operator<<(std::ostream& output, const ChainResidueAtomDescriptor& descriptor)
{
	output << descriptor.str();
	return output;
}

inline std::istream& operator>>(std::istream& input, ChainResidueAtomDescriptor& descriptor)
{
	std::string str;
	input >> str;
	descriptor=ChainResidueAtomDescriptor::from_str(str);
	if(!descriptor.valid())
	{
		input.setstate(std::ios::failbit);
	}
	return input;
}

class ChainResidueAtomDescriptorsPair
{
public:
	ChainResidueAtomDescriptor a;
	ChainResidueAtomDescriptor b;
	bool reversed_display;

	ChainResidueAtomDescriptorsPair() : reversed_display(false)
	{
	}

	ChainResidueAtomDescriptorsPair(const ChainResidueAtomDescriptor& a, const ChainResidueAtomDescriptor& b) : a(a<b ? a : b), b(a<b ? b : a), reversed_display(false)
	{
	}

	ChainResidueAtomDescriptorsPair(const ChainResidueAtomDescriptor& a, const ChainResidueAtomDescriptor& b, const bool reversed_display) : a(a<b ? a : b), b(a<b ? b : a), reversed_display(reversed_display)
	{
	}

	bool operator==(const ChainResidueAtomDescriptorsPair& v) const
	{
		return (a==v.a && b==v.b);
	}

	bool operator<(const ChainResidueAtomDescriptorsPair& v) const
	{
		return ((a<v.a) || (a==v.a && b<v.b));
	}

	bool contains(const ChainResidueAtomDescriptor& v) const
	{
		return (a==v || b==v);
	}
};

inline std::ostream& operator<<(std::ostream& output, const ChainResidueAtomDescriptorsPair& descriptors_pair)
{
	if(descriptors_pair.reversed_display)
	{
		output << descriptors_pair.b << " " << descriptors_pair.a;
	}
	else
	{
		output << descriptors_pair.a << " " << descriptors_pair.b;
	}
	return output;
}

inline std::istream& operator>>(std::istream& input, ChainResidueAtomDescriptorsPair& descriptors_pair)
{
	ChainResidueAtomDescriptor a;
	ChainResidueAtomDescriptor b;
	input >> a >> b;
	descriptors_pair=ChainResidueAtomDescriptorsPair(a, b);
	return input;
}

class ChainResidueAtomDescriptorsGraphOperations
{
public:
	template<typename T>
	static std::map<ChainResidueAtomDescriptor, T> accumulate_mapped_values_by_graph_neighbors(
			const std::map< ChainResidueAtomDescriptorsPair, T >& map_of_pair_descriptors,
			const int depth)
	{
		return construct_map_of_single_descriptors(map_of_pair_descriptors, construct_graph(map_of_pair_descriptors, depth));
	}

private:
	template<typename T>
	static std::map< ChainResidueAtomDescriptor, std::set<ChainResidueAtomDescriptor> > construct_graph(
			const std::map<ChainResidueAtomDescriptorsPair, T>& map_of_pair_descriptors,
			const int depth)
	{
		typedef ChainResidueAtomDescriptor CRAD;
		std::map< CRAD, std::set<CRAD> > graph;
		if(depth>0)
		{
			for(typename std::map<ChainResidueAtomDescriptorsPair, T>::const_iterator it=map_of_pair_descriptors.begin();it!=map_of_pair_descriptors.end();++it)
			{
				const ChainResidueAtomDescriptorsPair& crads=it->first;
				if(!(crads.a==crads.b || crads.a==CRAD::solvent() || crads.b==CRAD::solvent()))
				{
					graph[crads.a].insert(crads.b);
					graph[crads.b].insert(crads.a);
				}
			}
			for(int i=1;i<depth;i++)
			{
				std::map< CRAD, std::set<CRAD> > expanded_graph=graph;
				for(std::map< CRAD, std::set<CRAD> >::const_iterator graph_it=graph.begin();graph_it!=graph.end();++graph_it)
				{
					const CRAD& center=graph_it->first;
					const std::set<CRAD>& neighbors=graph_it->second;
					std::set<CRAD>& expandable_neighbors=expanded_graph[center];
					for(std::set<CRAD>::const_iterator neighbors_it=neighbors.begin();neighbors_it!=neighbors.end();++neighbors_it)
					{
						const std::set<CRAD>& neighbor_neighbors=graph[*neighbors_it];
						expandable_neighbors.insert(neighbor_neighbors.begin(), neighbor_neighbors.end());
					}
					expandable_neighbors.erase(center);
				}
				graph=expanded_graph;
			}
		}
		return graph;
	}

	template<typename T>
	static std::map<ChainResidueAtomDescriptor, T> construct_map_of_single_descriptors(
			const std::map< ChainResidueAtomDescriptorsPair, T >& map_of_pair_descriptors,
			const std::map< ChainResidueAtomDescriptor, std::set<ChainResidueAtomDescriptor> >& graph)
	{
		typedef ChainResidueAtomDescriptor CRAD;
		std::map<CRAD, T> map_of_single_descriptors;
		for(typename std::map<ChainResidueAtomDescriptorsPair, T>::const_iterator it=map_of_pair_descriptors.begin();it!=map_of_pair_descriptors.end();++it)
		{
			const ChainResidueAtomDescriptorsPair& crads=it->first;
			std::set<CRAD> related_crads;
			if(crads.a!=CRAD::solvent())
			{
				related_crads.insert(crads.a);
			}
			if(crads.b!=CRAD::solvent())
			{
				related_crads.insert(crads.b);
			}
			{
				const std::map< CRAD, std::set<CRAD> >::const_iterator graph_it=graph.find(crads.a);
				if(graph_it!=graph.end())
				{
					const std::set<CRAD>& related_crads1=graph_it->second;
					related_crads.insert(related_crads1.begin(), related_crads1.end());
				}
			}
			{
				const std::map< CRAD, std::set<CRAD> >::const_iterator graph_it=graph.find(crads.b);
				if(graph_it!=graph.end())
				{
					const std::set<CRAD>& related_crads2=graph_it->second;
					related_crads.insert(related_crads2.begin(), related_crads2.end());
				}
			}
			for(std::set<CRAD>::const_iterator jt=related_crads.begin();jt!=related_crads.end();++jt)
			{
				map_of_single_descriptors[*jt].add(it->second);
			}
		}
		return map_of_single_descriptors;
	}
};

class ChainResidueAtomDescriptorsSequenceOperations
{
public:
	static std::map<ChainResidueAtomDescriptor, double> smooth_residue_scores_along_sequence(const std::map<ChainResidueAtomDescriptor, double>& raw_scores, const unsigned int window)
	{
		if(window>0)
		{
			std::vector< std::pair<ChainResidueAtomDescriptor, double> > v(raw_scores.size());
			std::copy(raw_scores.begin(), raw_scores.end(), v.begin());
			std::vector< std::pair<ChainResidueAtomDescriptor, double> > sv=v;
			for(std::size_t i=0;i<v.size();i++)
			{
				const int start=std::max(0, (static_cast<int>(i)-static_cast<int>(window)));
				const int end=std::min(static_cast<int>(v.size())-1, (static_cast<int>(i)+static_cast<int>(window)));
				double sum_of_weighted_values=0.0;
				double sum_of_weights=0.0;
				for(int j=start;j<=end;j++)
				{
					if(v[i].first.chainID==v[j].first.chainID)
					{
						double ndist=fabs(static_cast<double>(static_cast<int>(i)-j))/static_cast<double>(window);
						double weight=(1.0-(ndist*ndist));
						sum_of_weights+=weight;
						sum_of_weighted_values+=v[j].second*weight;
					}
				}
				if(sum_of_weights>0.0)
				{
					sv[i].second=(sum_of_weighted_values/sum_of_weights);
				}
			}
			return std::map<ChainResidueAtomDescriptor, double>(sv.begin(), sv.end());
		}
		else
		{
			return raw_scores;
		}
	}
};

}

#endif /* AUXILIARIES_CHAIN_RESIDUE_ATOM_DESCRIPTOR_H_ */
