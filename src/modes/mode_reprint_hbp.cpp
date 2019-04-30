#include "../auxiliaries/program_options_handler.h"
#include "../auxiliaries/atoms_io.h"
#include "../auxiliaries/io_utilities.h"

#include "../common/chain_residue_atom_descriptor.h"

void reprint_hbp(const auxiliaries::ProgramOptionsHandler& poh)
{
	auxiliaries::ProgramOptionsHandlerWrapper pohw(poh);
	pohw.describe_io("stdin", true, false, "hbplus output");
	pohw.describe_io("stdout", false, true, "output");

	if(!pohw.assert_or_print_help(false))
	{
		return;
	}

	typedef common::ChainResidueAtomDescriptor CRAD;
	typedef common::ChainResidueAtomDescriptorsPair CRADsPair;

	std::set<CRADsPair> set_of_hbplus_crad_pairs;

	auxiliaries::AtomsIO::HBPlusReader::Data hbplus_file_data=auxiliaries::AtomsIO::HBPlusReader::read_data_from_file_stream(std::cin);
	if(!hbplus_file_data.hbplus_records.empty())
	{
		for(std::vector<auxiliaries::AtomsIO::HBPlusReader::HBPlusRecord>::const_iterator it=hbplus_file_data.hbplus_records.begin();it!=hbplus_file_data.hbplus_records.end();++it)
		{
			const auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& a=it->first;
			const auxiliaries::AtomsIO::HBPlusReader::ShortAtomDescriptor& b=it->second;
			const CRADsPair crads_pair(CRAD(CRAD::null_num(), a.chainID, a.resSeq, a.resName, a.name, "", ""), CRAD(CRAD::null_num(), b.chainID, b.resSeq, b.resName, b.name, "", ""));
			set_of_hbplus_crad_pairs.insert(crads_pair);
		}
	}

	auxiliaries::IOUtilities().write_set(set_of_hbplus_crad_pairs, std::cout);
}
