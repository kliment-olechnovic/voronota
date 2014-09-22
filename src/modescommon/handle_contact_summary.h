#ifndef MODESCOMMON_HANDLE_CONTACT_SUMMARY_H_
#define MODESCOMMON_HANDLE_CONTACT_SUMMARY_H_

#include "handle_contact.h"

namespace modescommon
{

struct ContactSummaryValue
{
	double area;
	long count;

	ContactSummaryValue() : area(0.0), count(0)
	{
	}

	void add(const ContactSummaryValue& v)
	{
		area+=v.area;
		count+=v.count;
	}

	void add(const ContactValue& v)
	{
		area+=v.area;
		count++;
	}
};

inline void print_contact_summary_record(const std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>& names, const ContactSummaryValue& value, std::ostream& output)
{
	output << names.first.str() << " " << names.second.str() << " " << value.area << " " << value.count << "\n";
}

inline bool add_contact_summary_record_from_stream_to_map(std::istream& input, std::map< std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor>, ContactSummaryValue >& map_of_records)
{
	std::pair<std::string, std::string> name_strings;
	ContactSummaryValue value;
	input >> name_strings.first >> name_strings.second >> value.area >> value.count;
	if(!input.fail() && !name_strings.first.empty() && !name_strings.second.empty())
	{
		const std::pair<auxiliaries::ChainResidueAtomDescriptor, auxiliaries::ChainResidueAtomDescriptor> names(auxiliaries::ChainResidueAtomDescriptor::from_str(name_strings.first), auxiliaries::ChainResidueAtomDescriptor::from_str(name_strings.second));
		if(names.first.valid() && names.second.valid())
		{
			map_of_records[refine_pair_by_ordering(names)].add(value);
			return true;
		}
	}
	return false;
}

}

#endif /* MODESCOMMON_HANDLE_CONTACT_SUMMARY_H_ */
