#pragma once
#include "Common.h"
#include "Input/UserInput.h"

// // Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
// enum Camera_Movement
// {
// 	FORWARD,
// 	BACKWARD,
// 	LEFT,
// 	RIGHT
// };

const float DEFAULT_FOV = 45.0f;
const float DEFAULT_ASPECT = (float)window_X / (float)window_Y;
const float DEFAULT_NEAR = 0.1f;
const float DEFAULT_FAR = 100.0f;

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

struct CameraConfiguration
{
	float fov;
	float aspect;
	float near;
	float far;
};

class Camera
{
public:
	Camera(const CameraConfiguration &config) : m_Config(config) {};
	~Camera() = default;

	glm::mat4 GetViewMatrix() const
	{
		return glm::lookAt(m_Position, m_Target, m_Up);
	}

	glm::mat4 GetProjectionMatrix() const
	{
		return glm::perspective(glm::radians(m_Config.fov), m_Config.aspect, m_Config.near, m_Config.far);
	};


	//move the camera around a target point based on user input
    void orbitControl(InputManager *inputManager, float deltaTime);

    // move the camera freely based on user input
	void flyControl(InputManager *inputManager, float deltaTime);

	// glm::vec3 getRight() const {
	// 	return glm::normalize(glm::cross(m_Target - m_Position, m_Up));
	// }

	// TODO: Should these be private?	
	glm::vec3 m_Position;
	glm::vec3 m_Up;
	glm::vec3 m_Target;
	// glm::vec3 m_Forward; // forward can be derived from position and target
	
	glm::vec3 WorldUp;
	CameraConfiguration m_Config;

	private:
	// Changes m_config.fov based on scroll input
	void zoomCamera(InputManager *inputManager);
	void pointCameraWithMouse(InputManager *inputManager, glm::vec3 &offset);
	void pointCameraWithMouseFly(InputManager *inputManager);
};
