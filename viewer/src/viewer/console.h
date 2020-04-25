#ifndef VIEWER_CONSOLE_H_
#define VIEWER_CONSOLE_H_

#include <string>
#include <list>
#include <vector>
#include <deque>

#include "../dependencies/imgui/imgui_impl_glfw_gl3.h"
#include "../duktaper/stocked_data_resources.h"

namespace voronota
{

namespace viewer
{

class Console
{
public:
	struct OutputToken
	{
		std::string content;
		float r;
		float g;
		float b;

		OutputToken(const std::string& content, const float r, const float g, const float b) : content(content), r(r), g(g), b(b)
		{
		}
	};

	struct ObjectState
	{
		std::string name;
		bool picked;
		bool visible;
	};

	static Console& instance()
	{
		static Console console;
		return console;
	}

	void set_need_keyboard_focus_in_command_input(const bool status)
	{
		command_line_interface_state_.need_keyboard_focus_in_command_input_=(status && !script_editor_state_.focused);
	}

	void set_next_prefix(const std::string& prefix)
	{
		command_line_interface_state_.next_prefix_=prefix;
	}

	void set_object_states(const std::vector<ObjectState>& object_states)
	{
		object_list_viewer_state_.object_states_=object_states;
	}

	void add_output(const std::string& content, const float r, const float g, const float b)
	{
		command_line_interface_state_.outputs_.push_back(OutputToken(content, r, g, b));
		if(command_line_interface_state_.outputs_.size()>50)
		{
			command_line_interface_state_.outputs_.pop_front();
		}
		command_line_interface_state_.scroll_output_=true;
	}

	void add_output_separator()
	{
		if(!command_line_interface_state_.outputs_.empty() && command_line_interface_state_.outputs_.back().content!=separator_string())
		{
			add_output(separator_string(), 0.0f, 0.0f, 0.0f);
		}
	}

	void add_history_output(const std::size_t n)
	{
		if(!command_line_interface_state_.history_of_commands_.empty())
		{
			const std::size_t first_i=((n>0 && n<command_line_interface_state_.history_of_commands_.size()) ? (command_line_interface_state_.history_of_commands_.size()-n) : 0);
			std::ostringstream output;
			for(std::size_t i=first_i;i<command_line_interface_state_.history_of_commands_.size();i++)
			{
				output << command_line_interface_state_.history_of_commands_[i] << "\n";
			}
			add_output(output.str(), 0.75f, 0.50f, 0.0f);
		}
	}

	void clear_outputs()
	{
		command_line_interface_state_.outputs_.clear();
		command_line_interface_state_.scroll_output_=true;
	}

	void clear_last_output()
	{
		if(!command_line_interface_state_.outputs_.empty() && command_line_interface_state_.outputs_.back().content==separator_string())
		{
			command_line_interface_state_.outputs_.pop_back();
		}
		if(!command_line_interface_state_.outputs_.empty())
		{
			command_line_interface_state_.outputs_.pop_back();
		}
		command_line_interface_state_.scroll_output_=true;
	}

	void set_documentation(const std::map<std::string, std::string>& documentation)
	{
		documentation_viewer_state_.documentation=documentation;
	}

	std::string execute(
			const int x_pos, const int y_pos,
			const int recommended_width,
			const int recommended_height,
			const int min_width, const int max_width,
			const int min_height, const int max_height)
	{
		std::string result;

		{
			ImGui::SetNextWindowPos(ImVec2(x_pos, y_pos));
			ImGui::SetNextWindowSizeConstraints(ImVec2(min_width, min_height), ImVec2(max_width, max_height));

			if(!ImGui::Begin("Console", 0, ImVec2(recommended_width, recommended_height), 0.5f, ImGuiWindowFlags_ShowBorders|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings))
			{
				current_width_=max_width;
				current_heigth_=ImGui::GetWindowHeight();
				ImGui::End();
				return result;
			}

			command_line_interface_state_.execute(result);

			current_width_=ImGui::GetWindowWidth();
			current_heigth_=ImGui::GetWindowHeight();

			ImGui::End();
		}

		{
			ImGui::SetNextWindowPos(ImVec2(current_width_, 0));

			ImVec2 panel_size(max_width-current_width_, max_height);
			ImGui::SetNextWindowSizeConstraints(panel_size, panel_size);

			if(!ImGui::Begin("Panel", 0, panel_size, 0.5f, ImGuiWindowFlags_ShowBorders|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize))
			{
				ImGui::End();
				return result;
			}

			if(ImGui::CollapsingHeader("Script editor##header_for_script_editor"))
			{
				script_editor_state_.execute(result);
			}
			else
			{
				script_editor_state_.focused=false;
			}

			if(ImGui::CollapsingHeader("Commands reference##header_for_list_of_documentation"))
			{
				documentation_viewer_state_.execute();
			}

			if(ImGui::CollapsingHeader("Objects##header_for_list_of_objects", ImGuiTreeNodeFlags_DefaultOpen))
			{
				object_list_viewer_state_.execute(result);
			}

			ImGui::End();
		}

		return result;
	}

	int current_width() const
	{
		return static_cast<int>(current_width_);
	}

	int current_heigth() const
	{
		return static_cast<int>(current_heigth_);
	}

private:
	struct CommandLineInterfaceState
	{
		std::deque<OutputToken> outputs_;
		std::vector<std::string> history_of_commands_;
		std::vector<std::string> dynamic_history_of_commands_;
		std::vector<char> command_buffer_;
		std::string next_prefix_;
		std::size_t index_of_history_of_commands_;
		bool scroll_output_;
		bool need_keyboard_focus_in_command_input_;

		CommandLineInterfaceState() :
			command_buffer_(1024, 0),
			index_of_history_of_commands_(0),
			scroll_output_(false),
			need_keyboard_focus_in_command_input_(false)
		{
		}

		static int on_command_input_data_request(ImGuiTextEditCallbackData* data)
		{
			CommandLineInterfaceState* obj=static_cast<CommandLineInterfaceState*>(data->UserData);
			return obj->handle_command_input_data_request(data);
		}

		void execute(std::string& result)
		{
			{
				ImGui::BeginChild("##console_scrolling_region", ImVec2(0,-ImGui::GetItemsLineHeightWithSpacing()));
				ImGui::PushItemWidth(-1);
				ImGui::PushTextWrapPos();
				for(std::size_t i=0;i<outputs_.size();i++)
				{
					const OutputToken& ot=outputs_[i];
					if(ot.content==separator_string())
					{
						ImGui::Separator();
					}
					else
					{
						ImVec4 color_text=ImVec4(ot.r, ot.g, ot.b, 1.0f);
						ImGui::PushStyleColor(ImGuiCol_Text, color_text);
						if(ot.content.size()<10000)
						{
							ImGui::TextUnformatted(ot.content.c_str());
						}
						else
						{
							ImGui::TextUnformatted(ot.content.c_str(), &(ot.content.c_str()[4000]));
							ImGui::TextUnformatted("------------------------------");
							ImGui::TextUnformatted("------------ skip ------------");
							ImGui::TextUnformatted("------------------------------");
							ImGui::TextUnformatted(&(ot.content.c_str()[ot.content.size()-4000]), &(ot.content.c_str()[ot.content.size()-1]));
						}
						ImGui::PopStyleColor();
					}
				}
				ImGui::PopTextWrapPos();
				ImGui::PopItemWidth();
				if(scroll_output_)
				{
					ImGui::SetScrollHere();
				}
				scroll_output_=false;
				ImGui::EndChild();
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				ImVec4 color_background=ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
				ImGui::PushItemWidth(-1);
				if(ImGui::InputText("##console_command_input", command_buffer_.data(), command_buffer_.size(), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackHistory|ImGuiInputTextFlags_CallbackAlways, &on_command_input_data_request, this))
				{
					result=(std::string(command_buffer_.data()));
					update_history_of_commands(result);
					command_buffer_.assign(command_buffer_.size(), 0);
					need_keyboard_focus_in_command_input_=true;
				}
				if(need_keyboard_focus_in_command_input_)
				{
					ImGui::SetKeyboardFocusHere(-1);
					need_keyboard_focus_in_command_input_=false;
				}
				ImGui::PopItemWidth();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
		}

		int handle_command_input_data_request(ImGuiTextEditCallbackData* data)
		{
			if(
					!history_of_commands_.empty() &&
					data->EventFlag==ImGuiInputTextFlags_CallbackHistory &&
					(data->EventKey==ImGuiKey_UpArrow || data->EventKey==ImGuiKey_DownArrow) &&
					data->BufSize>0
				)
			{
				if(dynamic_history_of_commands_.empty())
				{
					index_of_history_of_commands_=0;
					dynamic_history_of_commands_.push_back(std::string(data->Buf));
					dynamic_history_of_commands_.insert(dynamic_history_of_commands_.end(), history_of_commands_.rbegin(), history_of_commands_.rend());
				}
				else
				{
					if(index_of_history_of_commands_<dynamic_history_of_commands_.size())
					{
						dynamic_history_of_commands_[index_of_history_of_commands_]=std::string(data->Buf);
					}
					else
					{
						index_of_history_of_commands_=0;
					}
				}

				if(data->EventKey==ImGuiKey_UpArrow)
				{
					if(index_of_history_of_commands_+1<dynamic_history_of_commands_.size())
					{
						index_of_history_of_commands_++;
					}
				}
				else if(data->EventKey==ImGuiKey_DownArrow)
				{
					if(index_of_history_of_commands_>0)
					{
						index_of_history_of_commands_--;
					}
				}

				{
					std::string val=dynamic_history_of_commands_[index_of_history_of_commands_];
					if(val.size()>static_cast<std::size_t>(data->BufSize-1))
					{
						val=val.substr(0, static_cast<std::size_t>(data->BufSize-1));
					}
					for(size_t i=0;i<=val.size();i++)
					{
						data->Buf[i]=val.c_str()[i];
					}
					data->BufDirty=true;
					data->BufTextLen=static_cast<int>(val.size());
					data->CursorPos=data->BufTextLen;
					data->SelectionStart=data->BufTextLen;
					data->SelectionEnd=data->BufTextLen;
				}
			}
			else if(
					data->EventFlag==ImGuiInputTextFlags_CallbackAlways &&
					data->BufTextLen==0 &&
					!next_prefix_.empty() &&
					(next_prefix_.size()+2)<static_cast<std::size_t>(data->BufSize)
				)
			{
				for(size_t i=0;i<=next_prefix_.size();i++)
				{
					data->Buf[i]=next_prefix_.c_str()[i];
				}
				data->BufDirty=true;
				data->BufTextLen=static_cast<int>(next_prefix_.size());
				data->CursorPos=data->BufTextLen;
				data->SelectionStart=data->BufTextLen;
				data->SelectionEnd=data->BufTextLen;
				next_prefix_.clear();
			}
			return 0;
		}

		void update_history_of_commands(const std::string& command)
		{
			if(!command.empty() && (history_of_commands_.empty() || command!=history_of_commands_.back()))
			{
				history_of_commands_.push_back(command);
			}
			dynamic_history_of_commands_.clear();
		}
	};

	struct ScriptEditorState
	{
		std::vector<char> multiline_command_buffer_;
		bool focused;

		ScriptEditorState() :
			multiline_command_buffer_(16384, 0),
			focused(false)
		{
		}

		void execute(std::string& result)
		{
			if(ImGui::Button("Run", ImVec2(100,0)))
			{
				result=(std::string(multiline_command_buffer_.data()));
			}

			ImGui::SameLine();

			if(ImGui::Button("Clear", ImVec2(100,0)))
			{
				multiline_command_buffer_[0]=0;
			}

			ImGui::SameLine();

			{
				ImGui::Button("Load example", ImVec2(100,0));
				if(ImGui::BeginPopupContextItem("load example script context menu", 0))
				{
					for(std::size_t i=0;i<example_scripts().size();i++)
					{
						const std::string& script_name=example_scripts()[i].first;
						const std::string& script_body=example_scripts()[i].second;
						if(ImGui::Selectable(script_name.c_str()))
						{
							write_string_to_vector(script_body, multiline_command_buffer_);
						}
					}
					ImGui::EndPopup();
				}
			}

			ImGui::BeginChild("##script_editor_scrolling_region", ImVec2(0, 300));
			ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec4 color_background=ImVec4(0.25f, 0.25f, 0.25f, 0.4f);
			ImGui::PushStyleColor(ImGuiCol_Text, color_text);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
			ImGui::PushItemWidth(-1);
			ImGui::InputTextMultiline("##script_editor_input", multiline_command_buffer_.data(), multiline_command_buffer_.size(), ImVec2(-1,-1), ImGuiInputTextFlags_AllowTabInput);
			focused=ImGui::IsItemActive();
			ImGui::PopItemWidth();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::EndChild();
		}
	};

	struct DocumentationViewerState
	{
		std::map<std::string, std::string> documentation;

		DocumentationViewerState()
		{
		}

		void execute()
		{
			ImGui::BeginChild("##area_for_list_of_documentation", ImVec2(0, 400));

			for(std::map<std::string, std::string>::const_iterator it=documentation.begin();it!=documentation.end();++it)
			{
				const std::string& title=it->first;
				const std::string& content=it->second;
				if(ImGui::TreeNode(title.c_str()))
				{
					if(content.empty())
					{
						ImGui::TextUnformatted("    no arguments");
					}
					else
					{
						ImGui::TextUnformatted(content.c_str());
					}
					ImGui::TreePop();
				}
			}

			ImGui::EndChild();
		}
	};

	struct ObjectListViewerState
	{
		std::vector<ObjectState> object_states_;

		ObjectListViewerState()
		{
		}

		void execute(std::string& result) const
		{
			if(object_states_.empty())
			{
				ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				ImGui::TextUnformatted("No objects loaded yet.");
				ImGui::PopStyleColor();
				return;
			}

			{
				ObjectState summary;
				summary.picked=false;
				summary.visible=false;
				for(std::size_t i=0;i<object_states_.size();i++)
				{
					const ObjectState& os=object_states_[i];
					summary.picked=(summary.picked || os.picked);
					summary.visible=(summary.visible || os.visible);
				}
				{
					const std::string checkbox_id=std::string("##checkbox_pick");
					bool picked=summary.picked;
					if(ImGui::Checkbox(checkbox_id.c_str(), &picked))
					{
						if(picked)
						{
							result="pick-objects";
						}
						else
						{
							result="unpick-objects";
						}
					}
				}
				ImGui::SameLine();
				{
					const std::string checkbox_id=std::string("##checkbox_show");
					bool visible=summary.visible;
					if(ImGui::Checkbox(checkbox_id.c_str(), &visible))
					{
						if(visible)
						{
							result="show-objects";
						}
						else
						{
							result="hide-objects";
						}
					}
				}
				ImGui::SameLine();
				{
					const std::string button_id=std::string("z##button_zoom");
					if(ImGui::Button(button_id.c_str(), ImVec2(20,0)))
					{
						result="zoom-by-objects";
					}
				}
			}

			ImGui::Separator();

			for(std::size_t i=0;i<object_states_.size();i++)
			{
				const ObjectState& os=object_states_[i];
				{
					const std::string checkbox_id=std::string("##checkbox_pick_")+os.name;
					bool picked=os.picked;
					if(ImGui::Checkbox(checkbox_id.c_str(), &picked))
					{
						if(picked)
						{
							result=std::string("pick-more-objects -names ")+os.name;
						}
						else
						{
							result=std::string("unpick-objects -names ")+os.name;
						}
					}
				}
				ImGui::SameLine();
				{
					const std::string checkbox_id=std::string("##checkbox_show_")+os.name;
					bool visible=os.visible;
					if(ImGui::Checkbox(checkbox_id.c_str(), &visible))
					{
						if(visible)
						{
							result=std::string("show-objects -names ")+os.name;
						}
						else
						{
							result=std::string("hide-objects -names ")+os.name;
						}
					}
				}
				ImGui::SameLine();
				{
					const std::string button_id=std::string("z##button_zoom_")+os.name;
					if(ImGui::Button(button_id.c_str(), ImVec2(20,0)))
					{
						result=std::string("zoom-by-objects -names ")+os.name;
					}
				}
				ImGui::SameLine();
				{
					ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Text, color_text);
					ImGui::TextUnformatted(os.name.c_str());
					ImGui::PopStyleColor();
				}
			}
		}
	};

	Console() :
		current_width_(0.0f),
		current_heigth_(0.0f)
	{
	}

	static const std::string& separator_string()
	{
		static std::string str="---";
		return str;
	}

	static void write_string_to_vector(const std::string& str, std::vector<char>& v)
	{
		for(std::size_t i=0;i<str.size() && (i+1)<v.size();i++)
		{
			v[i]=str[i];
			v[i+1]=0;
		}
	}

	static const std::vector< std::pair<std::string, std::string> >& example_scripts()
	{
		static std::vector< std::pair<std::string, std::string> > collection;
		if(collection.empty())
		{
			std::istringstream input(duktaper::resources::data_script_examples());
			while(input.good())
			{
				std::string line;
				std::getline(input, line);
				if(line.size()>4 && line.rfind("### ", 0)==0)
				{
					collection.push_back(std::make_pair(line.substr(4), line));
					collection.back().second+="\n";
				}
				else if(!collection.empty())
				{
					collection.back().second+=line;
					collection.back().second+="\n";
				}
			}
		}
		return collection;
	}

	float current_width_;
	float current_heigth_;
	CommandLineInterfaceState command_line_interface_state_;
	ScriptEditorState script_editor_state_;
	DocumentationViewerState documentation_viewer_state_;
	ObjectListViewerState object_list_viewer_state_;
};

}

}

#endif /* VIEWER_CONSOLE_H_ */