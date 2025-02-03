#pragma once

#include <pad.h>

#include "../hooking/manager.h"
#include "../math.h"

namespace liborbisutil
{
	namespace pad
	{
		namespace state
		{
			enum hook
			{
				read,
				read_state,
				max
			};
		}

		namespace buttons
		{
			enum input_id
			{
				cross,
				square,
				circle,
				triangle,
				dpad_left,
				dpad_right,
				dpad_up,
				dpad_down,
				r1,
				r2,
				l1,
				l2,
				l3,
				r3,
				start,
				select,
				touchpad,
				max
			};
		}

		struct button_state
		{
			bool pressed;
			bool released;
			float input;
			float duration;
		};

		inline hook::manager detour_manager;

		inline hook::detour scePadRead_d;
		inline hook::detour scePadReadState_d;

		inline button_state current_buttons[buttons::max];
		inline button_state previous_buttons[buttons::max];

		inline std::function<void(ScePadData*, int)> callback[state::max];
		inline bool should_monitor[state::max];

		inline ScePadData current_pad_data;
		inline ScePadData previous_pad_data;

		inline uint32_t current_button_mask;
		inline uint32_t previous_button_mask;

		inline uint32_t pressed_button_mask;
		inline uint32_t released_button_mask;

		inline math::vector2 left_stick;
		inline math::vector2 right_stick;

		inline float l2_pressure;
		inline float r2_pressure;

		bool is_down(uint32_t button, float repeat_rate = 0.0f);
		bool is_pressed(uint32_t button);
		bool is_released(uint32_t button);

		void monitor(ScePadData* data);
		void register_callback(state::hook id, std::function<void(ScePadData*, int)> cb);
		void initialize(state::hook id, bool monitor, std::function<void(ScePadData*, int)> cb);
		void finalize();
	}
}