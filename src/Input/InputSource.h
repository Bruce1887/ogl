#pragma once
#include "Error.h"

#include <GLFW/glfw3.h>
#include <variant>

struct KeyboardUpdate
{
	int key, action, mods;
};
struct MousePosUpdate
{
	double x, y;
};
struct ScrollUpdate
{
	double xoffset, yoffset;
};
struct ButtonUpdate
{
	int button, action;
};

// #### The different input update data structures. These represent all the different inputs we want to register. ####
using InputUpdate = std::variant<
	KeyboardUpdate,
	MousePosUpdate,
	ScrollUpdate,
	ButtonUpdate>;

/**
 * @brief Base class for different input sources.
 *
 */
class InputSource
{
private:
	bool hasUnprocessedInput = false;

protected:
	void markUpdated() { hasUnprocessedInput = true; }
	void clearUpdated() { hasUnprocessedInput = false; }

public:
	virtual ~InputSource() = default;
	bool hasInput() const { return hasUnprocessedInput; }

	virtual void update(const InputUpdate &updateData) = 0;
};
