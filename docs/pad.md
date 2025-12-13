# Pad Module

Controller input handling and button state management.

## Button Constants

```cpp
using namespace liborbisutil::pad::buttons;

// Available buttons
cross, square, circle, triangle,
dpad_left, dpad_right, dpad_up, dpad_down,
r1, r2, l1, l2, l3, r3,
start, select, touchpad
```

## Checking Button State

```cpp
#include <liborbisutil.h>

using namespace liborbisutil::pad;

// Check if button is currently held down
if (is_down(SCE_PAD_BUTTON_CROSS)) {
    // Cross is being held
}

// With repeat rate (for menu navigation)
if (is_down(SCE_PAD_BUTTON_UP, 0.2f)) {
    // Triggers repeatedly while held
    menu_index--;
}

// Check if button was just pressed this frame
if (is_pressed(SCE_PAD_BUTTON_CIRCLE)) {
    // Circle was just pressed
}

// Check if button was just released
if (is_released(SCE_PAD_BUTTON_SQUARE)) {
    // Square was just released
}
```

## Analog Sticks

```cpp
// Get stick values (-1.0 to 1.0)
float leftX = liborbisutil::pad::get_left_stick_x();
float leftY = liborbisutil::pad::get_left_stick_y();

float rightX = liborbisutil::pad::get_right_stick_x();
float rightY = liborbisutil::pad::get_right_stick_y();

// Direct access to vectors
math::vector2 left = liborbisutil::pad::left_stick;
math::vector2 right = liborbisutil::pad::right_stick;
```

## Triggers

```cpp
// Get trigger pressure (0.0 to 1.0)
float l2 = liborbisutil::pad::l2_pressure;
float r2 = liborbisutil::pad::r2_pressure;
```

## Touchpad

```cpp
// Get touchpad touch state
touchpad_state touch0 = liborbisutil::pad::get_touchpad(0);
touchpad_state touch1 = liborbisutil::pad::get_touchpad(1);

if (touch0.id >= 0) {
    // Touch active
    float x = touch0.input.x;
    float y = touch0.input.y;
}
```

## Button State Structure

```cpp
struct button_state {
    bool pressed;    // Just pressed this frame
    bool released;   // Just released this frame
    float input;     // Analog value
    float duration;  // Time held
};

// Access current/previous state arrays
button_state current = liborbisutil::pad::current_buttons[buttons::cross];
button_state previous = liborbisutil::pad::previous_buttons[buttons::cross];
```

## Raw Pad Data

```cpp
// Access raw ScePadData
ScePadData current = liborbisutil::pad::current_pad_data;
ScePadData previous = liborbisutil::pad::previous_pad_data;

// Button masks
uint32_t currentMask = liborbisutil::pad::current_button_mask;
uint32_t pressedMask = liborbisutil::pad::pressed_button_mask;
uint32_t releasedMask = liborbisutil::pad::released_button_mask;
```

## Callbacks

Register callbacks for pad read hooks:

```cpp
// Set callback for pad reads
liborbisutil::pad::callback[state::read] = [](ScePadData* data, int count) {
    // Process or modify pad data
};

// Enable monitoring
liborbisutil::pad::should_monitor[state::read] = true;
```

## Example: Simple Input Handling

```cpp
void update_game() {
    using namespace liborbisutil::pad;
    
    // Movement
    player.velocity.x = get_left_stick_x() * player.speed;
    player.velocity.z = get_left_stick_y() * player.speed;
    
    // Camera
    camera.yaw += get_right_stick_x() * camera.sensitivity;
    camera.pitch += get_right_stick_y() * camera.sensitivity;
    
    // Actions
    if (is_pressed(SCE_PAD_BUTTON_CROSS)) {
        player.jump();
    }
    
    if (is_down(SCE_PAD_BUTTON_R2)) {
        player.accelerate(r2_pressure);
    }
}
```
