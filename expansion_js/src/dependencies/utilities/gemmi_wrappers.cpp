#include <set>

#include "../gemmi/cif.hpp"
#include "../gemmi/mmcif.hpp"

#include "../tinf/tinf_wrapper.h"

#include "gemmi_wrappers.h"

namespace gemmi_wrappers
{

std::vector<ModelRecord> read_uncompressed(const std::string& input)
{
	gemmi::cif::Document doc=gemmi::cif::read_string(input);
	gemmi::Structure structure=gemmi::make_structure(doc);
	std::vector<ModelRecord> model_records;
	for(std::vector<gemmi::Model>::const_iterator model_it=structure.models.begin();model_it!=structure.models.end();++model_it)
	{
		const gemmi::Model& model=(*model_it);
		ModelRecord model_record;
		model_record.name=model.name;
		for(std::vector<gemmi::Chain>::const_iterator chain_it=model.chains.begin();chain_it!=model.chains.end();++chain_it)
		{
			const gemmi::Chain& chain=(*chain_it);
			for(std::vector<gemmi::Residue>::const_iterator residue_it=chain.residues.begin();residue_it!=chain.residues.end();++residue_it)
			{
				const gemmi::Residue& residue=(*residue_it);
				for(std::vector<gemmi::Atom>::const_iterator atom_it=residue.atoms.begin();atom_it!=residue.atoms.end();++atom_it)
				{
					const gemmi::Atom& atom=(*atom_it);
					AtomRecord atom_record;

					atom_record.record_name=(residue.het_flag=='A' ? "ATOM" : (residue.het_flag=='H' ? "HETATM" : "ATOM"));

					atom_record.auth_chainID=chain.name;

					atom_record.resName=residue.name;

					atom_record.auth_resSeq_valid=residue.seqid.num.has_value();
					if(atom_record.auth_resSeq_valid)
					{
						atom_record.auth_resSeq=residue.seqid.num.value;
					}
					if(residue.seqid.has_icode())
					{
						atom_record.iCode=std::string(1, residue.seqid.icode);
					}

					atom_record.label_chainID=residue.subchain;

					atom_record.label_resSeq_valid=residue.label_seq.has_value();
					if(atom_record.label_resSeq_valid)
					{
						atom_record.label_resSeq=residue.label_seq.value;
					}

					atom_record.name=atom.name;
					atom_record.serial_valid=true;
					atom_record.serial=atom.serial;
					if(atom.has_altloc())
					{
						atom_record.altLoc=std::string(1, atom.altloc);
					}
					atom_record.x_valid=!std::isnan(atom.pos.x);
					if(atom_record.x_valid)
					{
						atom_record.x=atom.pos.x;
					}
					atom_record.y_valid=!std::isnan(atom.pos.y);
					if(atom_record.y_valid)
					{
						atom_record.y=atom.pos.y;
					}
					atom_record.z_valid=!std::isnan(atom.pos.z);
					if(atom_record.z_valid)
					{
						atom_record.z=atom.pos.z;
					}
					atom_record.occupancy_valid=!std::isnan(atom.occ);
					if(atom_record.occupancy_valid)
					{
						atom_record.occupancy=atom.occ;
					}
					atom_record.tempFactor_valid=!std::isnan(atom.b_iso);
					if(atom_record.tempFactor_valid)
					{
						atom_record.tempFactor=atom.b_iso;
					}
					atom_record.element=atom.element.name();
					atom_record.is_hydrogen=atom.element.is_hydrogen();
					model_record.atom_records.push_back(atom_record);
				}
			}
		}
		{
			std::set<int> set_of_serials;
			for(std::size_t i=0;i<model_record.atom_records.size();i++)
			{
				set_of_serials.insert(set_of_serials.end(), model_record.atom_records[i].serial);
			}
			if(set_of_serials.size()<model_record.atom_records.size())
			{
				for(std::size_t i=0;i<model_record.atom_records.size();i++)
				{
					model_record.atom_records[i].serial_valid=false;
				}
			}
		}
		model_records.push_back(model_record);
	}
	return model_records;
}

std::vector<ModelRecord> read(const std::string& input)
{
	if(TinfWrapper::check_if_string_gzipped(input))
	{
		std::string uncompressed_data;
		if(!TinfWrapper::uncompress_gzipped_string(input, uncompressed_data))
		{
			throw std::runtime_error(std::string("Failed to uncompress mmCIF file data."));
		}
		return read_uncompressed(uncompressed_data);
	}
	else
	{
		return read_uncompressed(input);
	}
}

std::vector<ModelRecord> read(std::istream& input)
{
	std::istreambuf_iterator<char> eos;
	std::string input_string(std::istreambuf_iterator<char>(input), eos);
	return read(input_string);
}

}
