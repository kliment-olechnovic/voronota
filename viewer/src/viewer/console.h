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
		core_state_.need_keyboard_focus_in_command_input_=(status && !script_editor_state_.opened_script_editor_);
	}

	void set_next_prefix(const std::string& prefix)
	{
		core_state_.next_prefix_=prefix;
	}

	void set_object_states(const std::vector<ObjectState>& object_states)
	{
		core_state_.object_states_=object_states;
	}

	void add_output(const std::string& content, const float r, const float g, const float b)
	{
		core_state_.outputs_.push_back(OutputToken(content, r, g, b));
		if(core_state_.outputs_.size()>50)
		{
			core_state_.outputs_.pop_front();
		}
		core_state_.scroll_output_=true;
	}

	void add_output_separator()
	{
		if(!core_state_.outputs_.empty() && core_state_.outputs_.back().content!=separator_string())
		{
			add_output(separator_string(), 0.0f, 0.0f, 0.0f);
		}
	}

	void add_history_output(const std::size_t n)
	{
		if(!core_state_.history_of_commands_.empty())
		{
			const std::size_t first_i=((n>0 && n<core_state_.history_of_commands_.size()) ? (core_state_.history_of_commands_.size()-n) : 0);
			std::ostringstream output;
			for(std::size_t i=first_i;i<core_state_.history_of_commands_.size();i++)
			{
				output << core_state_.history_of_commands_[i] << "\n";
			}
			add_output(output.str(), 0.75f, 0.50f, 0.0f);
		}
	}

	void clear_outputs()
	{
		core_state_.outputs_.clear();
		core_state_.scroll_output_=true;
	}

	void clear_last_output()
	{
		if(!core_state_.outputs_.empty() && core_state_.outputs_.back().content==separator_string())
		{
			core_state_.outputs_.pop_back();
		}
		if(!core_state_.outputs_.empty())
		{
			core_state_.outputs_.pop_back();
		}
		core_state_.scroll_output_=true;
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

			if(!ImGui::Begin("Console", 0, ImVec2(recommended_width, recommended_height), 0.5f, ImGuiWindowFlags_ShowBorders|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_MenuBar))
			{
				core_state_.current_width_=max_width;
				core_state_.current_heigth_=ImGui::GetWindowHeight();
				ImGui::End();
				return result;
			}

			if(ImGui::BeginMenuBar())
			{
				if(ImGui::BeginMenu("Windows"))
				{
					if(ImGui::MenuItem("Script editor"))
					{
						script_editor_state_.opened_script_editor_=true;
						script_editor_state_.need_keyboard_focus_in_script_editor_=true;
					}
					if(ImGui::MenuItem("Commands reference"))
					{
						documentation_viewer_state_.opened=true;
						documentation_viewer_state_.need_focus=true;
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			{
				ImGui::BeginChild("##console_scrolling_region", ImVec2(0,-ImGui::GetItemsLineHeightWithSpacing()));
				ImGui::PushItemWidth(-1);
				ImGui::PushTextWrapPos();
				for(std::size_t i=0;i<core_state_.outputs_.size();i++)
				{
					const OutputToken& ot=core_state_.outputs_[i];
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
				if(core_state_.scroll_output_)
				{
					ImGui::SetScrollHere();
				}
				core_state_.scroll_output_=false;
				ImGui::EndChild();
			}

			{
				ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				ImVec4 color_background=ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Text, color_text);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
				ImGui::PushItemWidth(-1);
				if(ImGui::InputText("##console_command_input", core_state_.command_buffer_.data(), core_state_.command_buffer_.size(), ImGuiInputTextFlags_EnterReturnsTrue|ImGuiInputTextFlags_CallbackHistory|ImGuiInputTextFlags_CallbackAlways, &on_command_input_data_request, &core_state_))
				{
					result=(std::string(core_state_.command_buffer_.data()));
					core_state_.update_history_of_commands(result);
					core_state_.command_buffer_.assign(core_state_.command_buffer_.size(), 0);
					core_state_.need_keyboard_focus_in_command_input_=true;
				}
				if(core_state_.need_keyboard_focus_in_command_input_)
				{
					ImGui::SetKeyboardFocusHere(-1);
					core_state_.need_keyboard_focus_in_command_input_=false;
				}
				ImGui::PopItemWidth();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}

			core_state_.current_width_=ImGui::GetWindowWidth();
			core_state_.current_heigth_=ImGui::GetWindowHeight();

			ImGui::End();
		}

		{
			ImGui::SetNextWindowPos(ImVec2(core_state_.current_width_, 0));

			ImVec2 panel_size(max_width-core_state_.current_width_, max_height);
			ImGui::SetNextWindowSizeConstraints(panel_size, panel_size);

			if(!ImGui::Begin("Panel", 0, panel_size, 0.5f, ImGuiWindowFlags_ShowBorders|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize))
			{
				core_state_.current_heigth_=ImGui::GetWindowHeight();
				ImGui::End();
				return result;
			}

			{
				ObjectState summary;
				summary.picked=false;
				summary.visible=false;
				for(std::size_t i=0;i<core_state_.object_states_.size();i++)
				{
					const ObjectState& os=core_state_.object_states_[i];
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

			for(std::size_t i=0;i<core_state_.object_states_.size();i++)
			{
				const ObjectState& os=core_state_.object_states_[i];
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

			ImGui::End();
		}

		{
			const std::string script_editor_result=script_editor_state_.execute();
			if(!script_editor_result.empty())
			{
				result=script_editor_result;
			}
		}

		documentation_viewer_state_.execute();

		return result;
	}

	int current_width() const
	{
		return static_cast<int>(core_state_.current_width_);
	}

	int current_heigth() const
	{
		return static_cast<int>(core_state_.current_heigth_);
	}

private:
	struct CoreState
	{
		std::deque<OutputToken> outputs_;
		std::vector<ObjectState> object_states_;
		std::vector<std::string> history_of_commands_;
		std::vector<std::string> dynamic_history_of_commands_;
		std::vector<char> command_buffer_;
		std::string next_prefix_;
		std::size_t index_of_history_of_commands_;
		bool scroll_output_;
		bool need_keyboard_focus_in_command_input_;
		float current_width_;
		float current_heigth_;

		CoreState() :
			command_buffer_(1024, 0),
			index_of_history_of_commands_(0),
			scroll_output_(false),
			need_keyboard_focus_in_command_input_(false),
			current_width_(0.0f),
			current_heigth_(0.0f)
		{
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
		bool opened_script_editor_;
		bool need_keyboard_focus_in_script_editor_;

		ScriptEditorState() :
			multiline_command_buffer_(16384, 0),
			opened_script_editor_(false),
			need_keyboard_focus_in_script_editor_(false)
		{
		}

		std::string execute()
		{
			std::string result;

			if(!opened_script_editor_)
			{
				return result;
			}

			if(!ImGui::Begin("Script editor", &opened_script_editor_, ImVec2(600, 400), 0.8f, ImGuiWindowFlags_ShowBorders|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_MenuBar))
			{
				ImGui::End();
				return result;
			}

			if(ImGui::BeginMenuBar())
			{
				if(ImGui::BeginMenu("Examples"))
				{
					for(std::size_t i=0;i<example_scripts().size();i++)
					{
						const std::string& script_name=example_scripts()[i].first;
						const std::string& script_body=example_scripts()[i].second;
						if(ImGui::MenuItem(script_name.c_str()))
						{
							write_string_to_vector(script_body, multiline_command_buffer_);
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::BeginChild("##script_editor_scrolling_region", ImVec2(0,-ImGui::GetItemsLineHeightWithSpacing()));
			ImVec4 color_text=ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImVec4 color_background=ImVec4(0.25f, 0.25f, 0.25f, 0.4f);
			ImGui::PushStyleColor(ImGuiCol_Text, color_text);
			ImGui::PushStyleColor(ImGuiCol_FrameBg, color_background);
			ImGui::PushItemWidth(-1);
			ImGui::InputTextMultiline("##script_editor_input", multiline_command_buffer_.data(), multiline_command_buffer_.size(), ImVec2(-1,-1), ImGuiInputTextFlags_AllowTabInput);
			if(need_keyboard_focus_in_script_editor_)
			{
				ImGui::SetKeyboardFocusHere(-1);
				need_keyboard_focus_in_script_editor_=false;
			}
			ImGui::PopItemWidth();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::EndChild();

			if(ImGui::Button("Run and close", ImVec2(100,0)))
			{
				result=(std::string(multiline_command_buffer_.data()));
				opened_script_editor_=false;
			}

			ImGui::SameLine();

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

			if(ImGui::Button("Close", ImVec2(100,0)))
			{
				opened_script_editor_=false;
			}

			ImGui::End();

			return result;
		}
	};

	struct DocumentationViewerState
	{
		std::map<std::string, std::string> documentation;
		bool opened;
		bool need_focus;

		DocumentationViewerState() :
			opened(false),
			need_focus(false)
		{
		}

		void execute()
		{
			if(!opened)
			{
				return;
			}

			if(!ImGui::Begin("Commands reference", &opened, ImVec2(600, 400), 0.8f, ImGuiWindowFlags_ShowBorders|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoCollapse))
			{
				ImGui::End();
				return;
			}

			for(std::map<std::string, std::string>::const_iterator it=documentation.begin();it!=documentation.end();++it)
			{
				const std::string& title=it->first;
				const std::string& content=it->second;
				if(ImGui::CollapsingHeader(title.c_str()))
				{
					ImGui::TextUnformatted(title.c_str());
					ImGui::TextUnformatted(content.c_str());
				}
			}

			if(need_focus)
			{
				ImGui::SetKeyboardFocusHere(-1);
				need_focus=false;
			}

			ImGui::End();
		}
	};

	Console()
	{
	}

	static int on_command_input_data_request(ImGuiTextEditCallbackData* data)
	{
		CoreState* obj=static_cast<CoreState*>(data->UserData);
		return obj->handle_command_input_data_request(data);
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

	CoreState core_state_;
	ScriptEditorState script_editor_state_;
	DocumentationViewerState documentation_viewer_state_;
};

}

}

#endif /* VIEWER_CONSOLE_H_ */
