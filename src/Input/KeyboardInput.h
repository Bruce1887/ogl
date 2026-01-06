#pragma once

#include "InputSource.h"
#include "Controls.h"
#include <utility>
#include <array>

/**
 * @brief class to hold all keyboard-related movement input state
 *
 * wasd: WASD input state.
 * shiftDown: true if the shift key is currently pressed, false otherwise.
 */
class MovementInput : public InputSource
{
private:
	// TODO: maybe more keys in future, like space for jump etc.
	/**
	 * @brief struct to hold WASD input state
	 */
	struct WasdInput
	{
		bool w_down;
		bool a_down;
		bool s_down;
		bool d_down;
	};
	WasdInput wasd;
	bool shiftDown;

public:
	// Call this method to update movement input state, and set hasUnprocessedInput to true. Intended to be called from a key callback.
	void update(const InputUpdate &updateData) override;
	void fetchMovement(int &outForward, int &outRight, bool &outShiftDown);
};

class KeyState
{
private:
	int m_key;
	bool m_value = false; // some value for the key. You can set this to be true when key is released or whatever you want to track.
	bool m_hasInput = false;

public:
	KeyState(int keyCode) : m_key(keyCode) {}

	int key() const { return m_key; }

	void update(bool isPressed)
	{
		m_value = isPressed;
		m_hasInput = true;
	}

	bool hasInput() const
	{
		return m_hasInput;
	}

	bool read()
	{
		return m_value;
	}


	bool readAndClear()
	{
		m_hasInput = false;
		return std::exchange(m_value, false); 
	}
};

class KeyboardInput
{
public:
	// Capture all keyboard movement input (WASD, Shift etc.)
	MovementInput movementInput;

	// All other key states can be added here as needed. i.e. if you want to register a key press for "K" key, add it here.
	inline static std::array keyStates = {
		KeyState{GLFW_KEY_ESCAPE},
		KeyState{OOGABOOGA_ATTACK_KEY},
		KeyState{OOGABOOGA_SPECIAL_ATTACK_KEY},};

	KeyState &getKeyState(int keyCode)
	{
		for (KeyState &ks : keyStates)
		{
			if (ks.key() == keyCode)
			{
				return ks;
			}
		}
#ifdef DEBUG
		assert(false && "KeyState for given keyCode not found!");
#endif
	}
};