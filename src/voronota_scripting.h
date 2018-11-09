#ifndef VORONOTA_SCRIPTING_H_
#define VORONOTA_SCRIPTING_H_

class ScriptingController
{
public:
	ScriptingController();

	virtual ~ScriptingController();

	const char* run(const char*);

private:
	void* implementation_;
};

#endif /* VORONOTA_SCRIPTING_H_ */
