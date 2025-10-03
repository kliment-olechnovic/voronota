#ifndef VIEWER_CONSOLE_TEXT_INTERFACE_INFO_H_
#define VIEWER_CONSOLE_TEXT_INTERFACE_INFO_H_

#include <string>
#include <vector>
#include <deque>

#include "../../dependencies/imgui/imgui.h"

namespace voronota
{

namespace viewer
{

namespace console
{

class CommandLineInterfacePanel;

class TextInterfaceInfo
{
public:
	struct OutputToken
	{
		float r;
		float g;
		float b;
		std::string content;

		OutputToken(const std::string& content, const float r, const float g, const float b) : r(r), g(g), b(b), content(content)
		{
		}
	};

	TextInterfaceInfo() :
		need_keyboard_focus_in_command_input(false),
		scroll_output(false),
		script_editor_focused(false)
	{
	}

	static const std::string& separator_string()
	{
		static std::string str="---";
		return str;
	}

	void set_need_keyboard_focus_in_command_input(const bool status)
	{
		need_keyboard_focus_in_command_input=(status && !script_editor_focused);
	}

	void set_script_editor_focused(const bool status)
	{
		script_editor_focused=status;
	}

	void set_next_prefix(const std::string& prefix)
	{
		next_prefix=prefix;
	}

	void add_output(const std::string& content, const float r, const float g, const float b)
	{
		outputs.push_back(OutputToken(content, r, g, b));
		if(outputs.size()>50)
		{
			outputs.pop_front();
		}
		scroll_output=true;
	}

	void add_output_separator()
	{
		if(!outputs.empty() && outputs.back().content!=separator_string())
		{
			add_output(separator_string(), 0.0f, 0.0f, 0.0f);
		}
	}

	void add_history_output(const std::size_t n)
	{
		if(!history_of_commands.empty())
		{
			const std::size_t first_i=((n>0 && n<history_of_commands.size()) ? (history_of_commands.size()-n) : 0);
			std::ostringstream output;
			for(std::size_t i=first_i;i<history_of_commands.size();i++)
			{
				output << history_of_commands[i] << "\n";
			}
			add_output(output.str(), 0.75f, 0.50f, 0.0f);
		}
	}

	void clear_outputs()
	{
		outputs.clear();
		scroll_output=true;
	}

	void clear_last_output()
	{
		if(!outputs.empty() && outputs.back().content==separator_string())
		{
			outputs.pop_back();
		}
		if(!outputs.empty())
		{
			outputs.pop_back();
		}
		scroll_output=true;
	}

	void add_clear_start()
	{
		add_output("clear_start", 0.50f, 0.50f, 0.50f);
	}

	void clear_outputs_from_last_clear_start()
	{
		while(!outputs.empty() && outputs.back().content!="clear_start")
		{
			outputs.pop_back();
		}
		if(!outputs.empty() && outputs.back().content=="clear_start")
		{
			outputs.pop_back();
		}
		scroll_output=true;
	}

private:
	std::deque<OutputToken> outputs;
	std::vector<std::string> history_of_commands;
	std::string next_prefix;
	bool need_keyboard_focus_in_command_input;
	bool scroll_output;
	bool script_editor_focused;

	friend class CommandLineInterfacePanel;
};

}

}

}


#endif /* VIEWER_CONSOLE_TEXT_INTERFACE_INFO_H_ */
