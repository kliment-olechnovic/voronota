#ifndef VIEWER_CONSOLE_MARKING_INFO_H_
#define VIEWER_CONSOLE_MARKING_INFO_H_

namespace voronota
{

namespace viewer
{

namespace console
{

class MarkingInfo
{
public:
	bool atoms_marking_updated;
	bool atoms_marking_present;
	bool contacts_marking_updated;
	bool contacts_marking_present;

	MarkingInfo() : atoms_marking_updated(false), atoms_marking_present(false), contacts_marking_updated(false), contacts_marking_present(false)
	{
	}

	void reset()
	{
		atoms_marking_updated=false;
		atoms_marking_present=false;
		contacts_marking_updated=false;
		contacts_marking_present=false;
	}
};

}

}

}

#endif /* VIEWER_CONSOLE_MARKING_INFO_H_ */
