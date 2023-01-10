#ifndef IMGUI_ADDONS_GLOBAL_TEXT_COLOR_VECTOR_H_
#define IMGUI_ADDONS_GLOBAL_TEXT_COLOR_VECTOR_H_

class ImGuiAddonGlobalTextColorVector
{
public:
	static inline bool active()
	{
		return mutable_state().active;
	}

	static inline unsigned int color(const int index, const unsigned int default_color)
	{
		return mutable_state().color(index, default_color);
	}

	ImGuiAddonGlobalTextColorVector(const unsigned int n, const unsigned int* colors)
	{
		mutable_state()=State(n, colors);
	}

	~ImGuiAddonGlobalTextColorVector()
	{
		mutable_state()=State();
	}

private:
	struct State
	{
		bool active;
		int colors_count;
		const unsigned int* colors_ptr;

		State() : active(false), colors_count(0), colors_ptr(0)
		{
		}

		State(const unsigned int n, const unsigned int* data) : active(false), colors_count(0), colors_ptr(0)
		{
			if(n>0 && data!=0)
			{
				active=true;
				colors_count=n;
				colors_ptr=data;
			}
		}

		inline unsigned int color(const int index, const unsigned int default_color) const
		{
			if(active && index<colors_count)
			{
				return colors_ptr[index];
			}
			return default_color;
		}
	};

	static inline State& mutable_state()
	{
		static State state;
		return state;
	}
};

#endif /* IMGUI_ADDONS_GLOBAL_TEXT_COLOR_VECTOR_H_ */
