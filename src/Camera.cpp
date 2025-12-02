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

	pointCameraWithMouse(inputManager, offset);
	zoomCamera(inputManager);
}

void Camera::flyControl(InputManager *inputManager, float deltaTime)
{
	// #### handle keyboard movement input ####
	int forwardMovement, rightMovement;
	bool shiftDown;
	inputManager->movementInput.fetchMovement(forwardMovement, rightMovement, shiftDown);

	float speed = 5.0f * deltaTime;
	if (shiftDown)
		speed *= 20.0f;

	glm::vec3 forward = glm::normalize(this->m_Target - this->m_Position);
	glm::vec3 right = glm::normalize(glm::cross(forward, this->m_Up));

	if (forwardMovement != 0)
	{
		float forwardMovementProduct = forwardMovement * speed;
		glm::vec3 newPos = forward * forwardMovementProduct;
		this->m_Position += newPos;
		this->m_Target += newPos;
	}
	if (rightMovement != 0)
	{
		float rightMovementProduct = rightMovement * speed;
		glm::vec3 newPos = right * rightMovementProduct;
		this->m_Position += newPos;
		this->m_Target += newPos;
	}

	pointCameraWithMouseFly(inputManager);
}

void Camera::pointCameraWithMouse(InputManager *inputManager, glm::vec3 &offset)
{
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
}

void Camera::pointCameraWithMouseFly(InputManager *inputManager)
{
	// #### handle mouse movement input for fly camera ####
	double mouseDeltaX = 0.0, mouseDeltaY = 0.0;
	

	// Temporary: Use Q/E and R/F keys to simulate mouse movement for testing without mouse input (mouse works poorly with WSL)
	float temp = 5.0;
	if (glfwGetKey(g_window, GLFW_KEY_Q) == GLFW_PRESS) {
		mouseDeltaX -= temp; 
	}
	if (glfwGetKey(g_window, GLFW_KEY_E) == GLFW_PRESS) {
		mouseDeltaX += temp; 
	}
	if (glfwGetKey(g_window, GLFW_KEY_R) == GLFW_PRESS) {		
		mouseDeltaY -= temp;
	}
	if (glfwGetKey(g_window, GLFW_KEY_F) == GLFW_PRESS) {
		mouseDeltaY += temp; 
	}

	// Check if there is any new mouse movement input that hasnt been processed
	// if (inputManager->mouseMoveInput.fetchDeltas(mouseDeltaX, mouseDeltaY)) // UNCOMMENT THIS IF YOU WANT MOUSE SUPPORT
	{
		float sensitivity = 0.005f; // adjust as necessary
		mouseDeltaX *= sensitivity;
		mouseDeltaY *= sensitivity * -1.0f; // Invert Y for typical fly camera behavior

		glm::vec3 forward = glm::normalize(this->m_Target - this->m_Position);
		glm::vec3 right = glm::normalize(glm::cross(forward, this->m_Up));
		// Horizontal rotation (around world up)
		glm::mat4 horizontalRotation = glm::rotate(glm::mat4(1.0f), (float)-mouseDeltaX, this->m_Up);
		forward = glm::vec3(horizontalRotation * glm::vec4(forward, 0.0f));

		// Vertical rotation (around right vector)
		glm::mat4 verticalRotation = glm::rotate(glm::mat4(1.0f), (float)mouseDeltaY, right);
		glm::vec3 newForward = glm::vec3(verticalRotation * glm::vec4(forward, 0.0f));
		// Prevent flipping over the top/bottom
		float angleFromUp = glm::degrees(glm::acos(glm::dot(glm::normalize(newForward), glm::normalize(this->m_Up))));
		if (angleFromUp > 5.0f && angleFromUp < 175.0f)
		{
			forward = newForward;
		}

		// Update target to maintain the distance from position
		float distance = glm::length(this->m_Target - this->m_Position);
		this->m_Target = this->m_Position + forward * distance;
	}
}

void Camera::zoomCamera(InputManager *inputManager)
{
	// #### handle mouse scroll input ####
	double scrollX, scrollY;
	// Check if there is any new scroll input that hasnt been processed
	if (inputManager->scrollInput.fetchScroll(scrollX, scrollY))
	{
		float zoomSensitivity = 1.0f; // adjust as necessary
		this->m_Config.fov = glm::clamp(this->m_Config.fov - (float)scrollY * zoomSensitivity, 1.0f, 90.0f);
	}
}
