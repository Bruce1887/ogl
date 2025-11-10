#include "Camera.h"

void cameraOrbitControl(Camera &camera, MovementInput movementInput, float deltaTime)
{

	float speed = 5.0f * deltaTime;
	if (movementInput.shiftDown)
		speed *= 5.0f;

	glm::vec3 offset = camera.m_Position - camera.m_Target;
	float distance = glm::length(offset);

	int right = movementInput.wasd.right;
	int forward = movementInput.wasd.forward;
	if (right != 0)
	{
		float angle = right * speed;
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
		offset = glm::vec3(rotation * glm::vec4(offset, 1.0f));
	}

	if (forward != 0)
	{
		distance -= forward * speed * 3.0f;
		distance = glm::max(distance, 1.0f); // Don't get too close
		offset = glm::normalize(offset) * distance;
	}

	camera.m_Position = camera.m_Target + offset;
}