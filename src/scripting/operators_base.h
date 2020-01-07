#ifndef SCRIPTING_OPERATORS_BASE_H_
#define SCRIPTING_OPERATORS_BASE_H_

#include "command_input.h"
#include "heterogeneous_storage.h"
#include "json_writer.h"
#include "command_documentation.h"

namespace scripting
{

namespace operators
{

template<class Operator>
class OperatorBase
{
public:
	OperatorBase()
	{
	}

	virtual ~OperatorBase()
	{
	}

	Operator& init(const std::string& args)
	{
		return underlying().init(CommandInput(std::string("noname ")+args));
	}

	CommandDocumentation document() const
	{
		CommandDocumentation doc;
		document(doc);
		return doc;
	}

	virtual void document(CommandDocumentation&) const
	{
	}

private:
	Operator& underlying()
	{
		return static_cast<Operator&>(*this);
	}
};

template<class OperatorResult>
class OperatorResultBase
{
public:
	OperatorResultBase()
	{
	}

	virtual ~OperatorResultBase()
	{
	}

	const OperatorResult& write(std::ostream& output) const
	{
		HeterogeneousStorage heterostorage;
		underlying().write(heterostorage);
		if(heterostorage.variant_object.empty())
		{
			return std::string();
		}
		JSONWriter::write(JSONWriter::Configuration(0), heterostorage.variant_object, output);
		return underlying();
	}

	std::string str() const
	{
		std::ostringstream output;
		write(output);
		return output.str();
	}

private:
	const OperatorResult& underlying() const
	{
		return static_cast<OperatorResult&>(*this);
	}
};

}

}

#endif /* SCRIPTING_OPERATORS_BASE_H_ */
