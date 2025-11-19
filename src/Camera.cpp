#include "Camera.h"

void Camera::orbitControl(InputManager *inputManager, float deltaTime)
{
	// #### handle keyboard movement input ####
	int forwardMovement, rightMovement;
	bool shiftDown;
	inputManager->movementInput.fetchMovement(forwardMovement, rightMovement, shiftDown);

	float speed = 5.0f * deltaTime;
	if (shiftDown)
		speed *= 5.0f;

	glm::vec3 offset = this->m_Position - this->m_Target;
	float distance = glm::length(offset);
	if (rightMovement != 0)
	{
		float angle = rightMovement * speed;
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, this->m_Up);
		offset = glm::vec3(rotation * glm::vec4(offset, 1.0f));
	}
	if (forwardMovement != 0)
	{
		distance -= forwardMovement * speed;
		distance = glm::max(distance, 1.0f); // Don't get too close
		offset = glm::normalize(offset) * distance;
	}

	this->m_Position = this->m_Target + offset;

	// #### handle mouse movement input ####
	double mouseDeltaX, mouseDeltaY;
	if (inputManager->mouseMoveInput.fetchDeltas(mouseDeltaX, mouseDeltaY))
	{
		float sensitivity = 0.02f; // adjust as necessary
		mouseDeltaX *= sensitivity;
		mouseDeltaY *= sensitivity;

		// Horizontal rotation (around world up)
		glm::mat4 horizontalRotation = glm::rotate(glm::mat4(1.0f), (float)-mouseDeltaX, this->m_Up);
		offset = glm::vec3(horizontalRotation * glm::vec4(offset, 1.0f));

		// Vertical rotation (around right vector)
		glm::vec3 right = glm::normalize(glm::cross(offset, this->m_Up));
		glm::mat4 verticalRotation = glm::rotate(glm::mat4(1.0f), (float)-mouseDeltaY, right);
		glm::vec3 newOffset = glm::vec3(verticalRotation * glm::vec4(offset, 1.0f));

		// Prevent flipping over the top/bottom
		float angleFromUp = glm::degrees(glm::acos(glm::dot(glm::normalize(newOffset), glm::normalize(this->m_Up))));
		if (angleFromUp > 5.0f && angleFromUp < 175.0f)
		{
			offset = newOffset;
		}
		this->m_Position = this->m_Target + offset;
	}

	// #### handle mouse scroll input ####
	double scrollX, scrollY;
	if (inputManager->scrollInput.fetchScroll(scrollX, scrollY))
	{		
		float zoomSensitivity = 1.0f; // adjust as necessary
		distance -= (float)scrollY * zoomSensitivity;
		this->m_Config.fov = glm::clamp(this->m_Config.fov - (float)scrollY * zoomSensitivity, 1.0f, 90.0f);
	}
}