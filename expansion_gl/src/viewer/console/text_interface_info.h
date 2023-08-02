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

class CommandLineInterfaceState;

class TextInterfaceInfo
{
public:
	struct OutputToken
	{
		float r;
		float g;
		float b;
		std::string content;
		std::vector<unsigned int> char_colors;

		OutputToken(const std::string& content, const float r, const float g, const float b) : r(r), g(g), b(b), content(content)
		{
			const ImU32 cui_default=0xFFAAAAAA;
			const ImU32 cui_string=ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, 1.0f));
			const ImU32 cui_number=0xFF55EE55;
			const ImU32 cui_key=0xFF55EEEE;
			const ImU32 cui_punctuation=0xFFCCCCCC;

			char_colors.resize(content.size(), cui_default);

			{
				int i=0;
				while(i<static_cast<int>(content.size()))
				{
					if(content[i]=='"')
					{
						char_colors[i]=cui_string;
						i++;
						bool ended=false;
						while(i<static_cast<int>(content.size()) && !ended)
						{
							if(content[i]=='"' && content[i-1]!='\\')
							{
								ended=true;
							}
							char_colors[i]=cui_string;
							i++;
						}
					}
					else
					{
						i++;
					}
				}
			}

			{
				int i=0;
				while(i<static_cast<int>(content.size()))
				{
					if(char_colors[i]==cui_default && ((content[i]>='0' && content[i]<='9') || (content[i]=='-' && (i+1)<static_cast<int>(content.size()) && char_colors[i+1]==cui_default && content[i+1]>='0' && content[i+1]<='9')))
					{
						char_colors[i]=cui_number;
						i++;
						bool ended=false;
						while(i<static_cast<int>(content.size()) && !ended)
						{
							if(char_colors[i]==cui_default && ((content[i]>='0' && content[i]<='9') || content[i]=='.'))
							{
								char_colors[i]=cui_number;
							}
							else
							{
								ended=true;
							}
							i++;
						}
					}
					else
					{
						i++;
					}
				}
			}

			for(int i=0;i<static_cast<int>(content.size());i++)
			{
				if(i>=2 && content[i]==':' && i>0 && content[i-1]=='"')
				{
					char_colors[i]=cui_punctuation;
					char_colors[i-1]=cui_key;
					int j=(i-2);
					for(;j>=0;j--)
					{
						char_colors[j]=cui_key;
						if(content[j]=='"')
						{
							j=-1;
						}
					}
				}
			}

			for(int i=0;i<static_cast<int>(content.size());i++)
			{
				if(char_colors[i]==cui_default)
				{
					const char c=content[i];
					if(c=='{' || c=='}' || c=='[' || c==']' || c==',')
					{
						char_colors[i]=cui_punctuation;
					}
				}
			}
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

private:
	std::deque<OutputToken> outputs;
	std::vector<std::string> history_of_commands;
	std::string next_prefix;
	bool need_keyboard_focus_in_command_input;
	bool scroll_output;
	bool script_editor_focused;

	friend class CommandLineInterfaceState;
};

}

}

}


#endif /* VIEWER_CONSOLE_TEXT_INTERFACE_INFO_H_ */
