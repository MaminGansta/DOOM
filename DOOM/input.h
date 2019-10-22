#pragma once

#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44

#define VK_DEBUG 0x50

struct Button_state
{
	bool is_down { false };
	bool changed { false };
};

enum
{
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_LROTATE,
	BUTTON_RROTATE,

	BUTTON_COUNT
};

struct Input
{
	Button_state buttons[BUTTON_COUNT];
};