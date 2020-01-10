#ifndef SCRIPTING_OPERATORS_IMPORT_ADJUNCTS_OF_ATOMS_H_
#define SCRIPTING_OPERATORS_IMPORT_ADJUNCTS_OF_ATOMS_H_

#include "../operators_common.h"

namespace voronota
{

namespace scripting
{

namespace operators
{

class ImportAdjunctsOfAtoms : public OperatorBase<ImportAdjunctsOfAtoms>
{
public:
	struct Result : public OperatorResultBase<Result>
	{
		std::vector<std::string> adjunct_names;

		void store(HeterogeneousStorage& heterostorage) const
		{
			std::vector<VariantValue>& array=heterostorage.variant_object.values_array("adjunct_names");
			for(std::size_t i=0;i<adjunct_names.size();i++)
			{
				array.push_back(VariantValue(adjunct_names[i]));
			}
		}
	};

	std::string file;
	bool no_serial;
	bool no_name;
	bool no_resSeq;
	bool no_resName;

	ImportAdjunctsOfAtoms() : no_serial(false), no_name(false), no_resSeq(false), no_resName(false)
	{
	}

	void initialize(CommandInput& input)
	{
		file=input.get_value_or_first_unused_unnamed_value("file");
		no_serial=input.get_flag("no-serial");
		no_name=input.get_flag("no-name");
		no_resSeq=input.get_flag("no-resSeq");
		no_resName=input.get_flag("no-resName");
	}

	void document(CommandDocumentation& doc) const
	{
		doc.set_option_decription(CDOD("file", CDOD::DATATYPE_STRING, "path to file"));
		doc.set_option_decription(CDOD("no-serial", CDOD::DATATYPE_BOOL, "flag to exclude atom serials"));
		doc.set_option_decription(CDOD("no-name", CDOD::DATATYPE_BOOL, "flag to exclude atom names"));
		doc.set_option_decription(CDOD("no-resSeq", CDOD::DATATYPE_BOOL, "flag to exclude residue sequence numbers"));
		doc.set_option_decription(CDOD("no-resName", CDOD::DATATYPE_BOOL, "flag to exclude residue names"));
	}

	Result run(DataManager& data_manager) const
	{
		data_manager.assert_atoms_availability();

		if(file.empty())
		{
			throw std::runtime_error(std::string("Empty input adjuncts file name."));
		}

		InputSelector finput_selector(file);
		std::istream& finput=finput_selector.stream();

		if(!finput.good())
		{
			throw std::runtime_error(std::string("Failed to read file '")+file+"'.");
		}

		std::vector<std::string> header;
		{
			std::string line;
			std::getline(finput, line);
			if(!line.empty())
			{
				std::istringstream linput(line);
				while(linput.good())
				{
					std::string token;
					linput >> token;
					if(!token.empty())
					{
						header.push_back(token);
					}
				}
			}
		}

		if(header.size()<2 || header[0]!="ID")
		{
			throw std::runtime_error(std::string("Invalid header in file."));
		}

		std::map< std::string, std::map<common::ChainResidueAtomDescriptor, double> > maps_of_adjuncts;

		while(finput.good())
		{
			std::string line;
			std::getline(finput, line);
			if(!line.empty())
			{
				std::istringstream linput(line);
				common::ChainResidueAtomDescriptor crad;
				linput >> crad;
				if(linput.fail() || !crad.valid())
				{
					throw std::runtime_error(std::string("Invalid ID in file."));
				}
				for(std::size_t i=1;i<header.size();i++)
				{
					std::string token;
					linput >> token;
					if(linput.fail() || token.empty())
					{
						throw std::runtime_error(std::string("Missing value in file."));
					}
					if(token!="NA")
					{
						std::istringstream vinput(token);
						double value=0.0;
						vinput >> value;
						if(vinput.fail())
						{
							throw std::runtime_error(std::string("Invalid value '")+token+"'.");
						}
						maps_of_adjuncts[header[i]][crad.without_some_info(no_serial, no_name, no_resSeq, no_resName)]=value;
					}
				}
			}
		}

		for(std::map< std::string, std::map<common::ChainResidueAtomDescriptor, double> >::const_iterator it=maps_of_adjuncts.begin();it!=maps_of_adjuncts.end();++it)
		{
			const std::string& adjunct_name=it->first;
			const std::map<common::ChainResidueAtomDescriptor, double>& adjunct_map=it->second;

			for(std::size_t i=0;i<data_manager.atoms().size();i++)
			{
				const Atom& atom=data_manager.atoms()[i];
				const std::pair<bool, double> value=common::MatchingUtilities::match_crad_with_map_of_crads(true, atom.crad, adjunct_map);
				if(value.first)
				{
					data_manager.atom_adjuncts_mutable(i)[adjunct_name]=value.second;
				}
			}
		}

		Result result;
		for(std::map< std::string, std::map<common::ChainResidueAtomDescriptor, double> >::const_iterator it=maps_of_adjuncts.begin();it!=maps_of_adjuncts.end();++it)
		{
			result.adjunct_names.push_back(it->first);
		}

		return result;
	}
};

}

}

}

#endif /* SCRIPTING_OPERATORS_IMPORT_ADJUNCTS_OF_ATOMS_H_ */
