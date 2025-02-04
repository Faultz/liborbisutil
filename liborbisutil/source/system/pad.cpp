#include "pad.h"

#include "resolve.h"
#include "logger.h"

namespace liborbisutil
{
	namespace pad
	{
		struct SceVideoOutBuffer
		{
			void* videoBuffer;
			size_t idk;
		};

		struct SceVideoOut
		{
			char _0x00[0x48];
			SceVideoOutBuffer buffers[5];	// 0x48
			char _0x98[0xF8];
			unsigned int formatBits;		// 0x190
			char _0x194[0x08];
			int width;						// 0x19C
			int height;						// 0x1A0
		};

		SceVideoOut* video_data;

		const uint32_t GAMEPAD_BUTTONS[] =
		{
			SCE_PAD_BUTTON_CROSS,
			SCE_PAD_BUTTON_SQUARE,
			SCE_PAD_BUTTON_CIRCLE,
			SCE_PAD_BUTTON_TRIANGLE,
			SCE_PAD_BUTTON_LEFT,
			SCE_PAD_BUTTON_RIGHT,
			SCE_PAD_BUTTON_UP,
			SCE_PAD_BUTTON_DOWN,
			SCE_PAD_BUTTON_R1,
			SCE_PAD_BUTTON_R2,
			SCE_PAD_BUTTON_L1,
			SCE_PAD_BUTTON_L2,
			SCE_PAD_BUTTON_L3,
			SCE_PAD_BUTTON_R3,
			SCE_PAD_BUTTON_START,
			SCE_PAD_BUTTON_INTERCEPTED,
			SCE_PAD_BUTTON_TOUCH_PAD
			/*	SCE_PAD_BUTTON_INTERCEPTED, */
		};

		int calc_repeat_amount(float t, float t_prev, float repeat_delay, float repeat_rate)
		{
			if (t == 0.0f)
				return 1;
			if (t <= repeat_delay || repeat_rate <= 0.0f)
				return 0;
			const int count = (int)((t - repeat_delay) / repeat_rate) - (int)((t_prev - repeat_delay) / repeat_rate);
			return (count > 0) ? count : 0;
		}

		bool is_down(uint32_t button, float repeat_rate)
		{
			const float t = current_buttons[button].duration;

			if (repeat_rate == 0.0f)
				return current_buttons[button].pressed;

			return (float)calc_repeat_amount(t, t - (1.f / 60.f), 0.f, repeat_rate) ? true : false;
		}

		bool is_pressed(uint32_t button)
		{
			return current_buttons[button].pressed;
		}

		bool is_released(uint32_t button)
		{
			return current_buttons[button].released;
		}

		math::vector2 calculateStickValue(int x, int y, float deadzone)
		{
			static const float m_recipMaxByteAsFloat = 1.0f / 255.0f;

			float valueX = static_cast<float>(x * 2 - 255) * m_recipMaxByteAsFloat;
			float valueY = static_cast<float>(y * 2 - 255) * m_recipMaxByteAsFloat;

			if (fabsf(valueX) < deadzone) valueX = 0.0f;
			if (fabsf(valueY) < deadzone) valueY = 0.0f;

			return math::vector2(valueX, valueY);
		}

		void updateTouchpadValues(ScePadData* padData)
		{
		}

		void monitor(ScePadData* padData)
		{
			if (!padData->connected)
				return;

			current_pad_data = *padData;

			memcpy(previous_buttons, current_buttons, sizeof(current_buttons));

			current_button_mask = current_pad_data.buttons;
			pressed_button_mask = current_button_mask & ~previous_button_mask;
			released_button_mask = ~current_button_mask & previous_button_mask;

			for (int i = 0; i < buttons::max; i++)
			{
				const uint32_t updateButtonMask = padData->buttons & GAMEPAD_BUTTONS[i];
				const uint32_t prevButtonMask = previous_pad_data.buttons & GAMEPAD_BUTTONS[i];

				const uint32_t pressed = (prevButtonMask ^ updateButtonMask) & updateButtonMask;
				const uint32_t released = (prevButtonMask ^ updateButtonMask) & ~updateButtonMask;

				current_buttons[i].pressed = pressed;
				current_buttons[i].released = released;
				current_buttons[i].input = (padData->buttons & GAMEPAD_BUTTONS[i]) ? 1.0f : 0.0f;
				current_buttons[i].duration = (current_buttons[i].input > 0.0f) ? (current_buttons[i].duration < 0.0f ? 0.0f : current_buttons[i].duration + (1.f / 60.f)) : -1.0f;
			}

			left_stick = calculateStickValue(padData->leftStick.x, padData->leftStick.y, 0.1f);
			right_stick = calculateStickValue(padData->rightStick.x, padData->rightStick.y, 0.1f);

			l2_pressure = static_cast<float>(padData->analogButtons.l2) / 255.0f;
			r2_pressure = static_cast<float>(padData->analogButtons.r2) / 255.0f;

			updateTouchpadValues(padData);

			previous_pad_data = current_pad_data;
			previous_button_mask = current_button_mask;
		}

		int scePadReadState(int handle, ScePadData* data)
		{
			int res = scePadReadState_d.invoke<int>(handle, data);

			if (handle != 0)
			{
				if (should_monitor[state::read_state])
					monitor(data);

				if (callback[state::read_state])
					callback[state::read_state](data, 1);
			}

			return res;
		}

		int scePadRead(int handle, ScePadData* data, int num)
		{
			int res = scePadRead_d.invoke<int>(handle, data, num);

			if (handle != 0)
			{
				if (should_monitor[state::read])
					monitor(data);

				if (callback[state::read])
					callback[state::read](data, num);
			}

			return res;
		}

		void register_callback(state::hook id, std::function<void(ScePadData*, int)> cb)
		{
			pad::callback[id] = cb;
		}

		void initialize(state::hook id, bool monitor, std::function<void(ScePadData*, int)> cb)
		{		
			
			// set callback to be used in the hook.
			callback[id] = cb;
			should_monitor[id] = monitor;

			auto videoOutBase = resolve::get_module_address("libSceVideoOut.sprx");
			static auto sceVideoOutGet = (void* (*)(int))(videoOutBase + 0xA600);
			int handle = *(int*)(videoOutBase + 0x1CAD0);
			video_data = *(SceVideoOut**)sceVideoOutGet(handle);

			std::string pad_hook_name = "";
			switch (id)
			{
			case state::read:
				pad_hook_name = "scePadRead";
				break;
			case state::read_state:
				pad_hook_name = "scePadReadState";
				break;
			default:
				break;
			}

			LOG_DEBUG("initializing pad hook: %s\n", pad_hook_name.data());

			auto scePadReadStateAddr = resolve::get_export_address("libScePad.sprx", pad_hook_name);

			switch (id)
			{
			case state::read:
				detour_manager.create("scePadRead", scePadReadStateAddr, scePadReadState);
				detour_manager.enable("scePadRead");
				scePadRead_d = *detour_manager.get("scePadRead");
				break;
			case state::read_state:
				detour_manager.create("scePadReadState", scePadReadStateAddr, scePadReadState);
				detour_manager.enable("scePadReadState");
				scePadReadState_d = *detour_manager.get("scePadReadState");
				break;
			default:
				break;
			}

			LOG_DEBUG("pad hook initialized: %s\n", pad_hook_name.data());
		}

		void finalize()
		{
			detour_manager.clear();
		}
	}
}