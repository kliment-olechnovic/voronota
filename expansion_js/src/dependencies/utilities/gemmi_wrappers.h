#ifndef DEPENDENCIES_UTILITIES_GEMMI_WRAPPERS_H_
#define DEPENDENCIES_UTILITIES_GEMMI_WRAPPERS_H_

#include <vector>
#include <string>
#include <iostream>

namespace gemmi_wrappers
{

struct AtomRecord
{
	std::string record_name;
	int serial;
	std::string name;
	std::string altLoc;
	std::string resName;
	std::string auth_chainID;
	int auth_resSeq;
	std::string iCode;
	std::string label_chainID;
	int label_resSeq;
	double x;
	double y;
	double z;
	double occupancy;
	double tempFactor;
	std::string element;

	bool is_hydrogen;
	bool serial_valid;
	bool auth_resSeq_valid;
	bool label_resSeq_valid;
	bool x_valid;
	bool y_valid;
	bool z_valid;
	bool occupancy_valid;
	bool tempFactor_valid;
};

struct ModelRecord
{
	std::string name;
	std::vector<AtomRecord> atom_records;
};

std::vector<ModelRecord> read(const std::string& input);

std::vector<ModelRecord> read(std::istream& input);

bool write_to_stream(const std::vector<ModelRecord>& model_records, std::ostream& output);

}

#endif /* DEPENDENCIES_UTILITIES_GEMMI_WRAPPERS_H_ */
