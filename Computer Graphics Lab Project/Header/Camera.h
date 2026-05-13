#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "InputManager.h"

enum class CameraMode
{
	THIRD_PERSON,
	AERIAL,
	INTEREST_POINT
};

class Camera
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat turnSpeed);

	void keyControl(const InputManager& input, GLfloat deltaTime);
	void mouseControl(GLfloat xDelta, GLfloat yDelta);

	glm::mat4 calculateViewMatrix() const;

	glm::vec3 getPosition() const { return position; }
	glm::vec3 getDirection() const { return glm::normalize(front); }

	// Modos de cámara
	void setCameraMode(CameraMode mode);
	CameraMode getCameraMode() const { return currentMode; }

	// Control de cámara de 3era persona (¡Añadido modelRotation!)
	void updateThirdPersonCamera(const glm::vec3& targetPosition, GLfloat modelRotation, GLfloat deltaTime);

	// Control de órbita
	void setAutoOrbit(bool enabled) { autoOrbit = enabled; }
	bool isAutoOrbitEnabled() const { return autoOrbit; }
	void rotateOrbit(GLfloat deltaAngle) { orbitAngle += deltaAngle; }
	void setOrbitSpeed(GLfloat speed) { orbitSpeed = speed; }
	GLfloat getOrbitAngle() const { return orbitAngle; }

	// Control de cámara aérea
	void updateAerialCamera(const InputManager& input, GLfloat deltaTime);

	// Control de puntos de interés
	void addInterestPoint(const glm::vec3& point, const glm::vec3& lookAt);
	void nextInterestPoint();
	void updateInterestPointCamera(GLfloat deltaTime);

	// Transición genérica entre modos
	void setTransitionTarget(const glm::vec3& targetPos, const glm::vec3& targetLookAt);
	bool isInModeTransition() const { return inModeTransition; }

	// ¡AQUÍ ESTÁ LA CORRECCIÓN! (Movido de private a public)
	bool updateModeTransition(GLfloat deltaTime);

	~Camera();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;
	GLfloat moveSpeed;
	GLfloat turnSpeed;

	// Para seguimiento suave
	glm::vec3 targetPosition;
	GLfloat smoothSpeed = 5.0f;
	GLfloat thirdPersonDistance = 2.5f;
	GLfloat thirdPersonHeight = 1.0f;
	GLfloat orbitAngle = 0.0f;
	GLfloat orbitSpeed = 45.0f;
	bool autoOrbit = false;
	GLfloat manualOrbitSensitivity = 100.0f;

	// Para cámara aérea
	GLfloat aerialHeight = 15.0f;
	GLfloat aerialMoveSpeed = 15.0f;
	glm::vec3 aerialPosition;

	// Para puntos de interés
	struct InterestPoint
	{
		glm::vec3 cameraPosition;
		glm::vec3 lookAtPoint;
	};
	std::vector<InterestPoint> interestPoints;
	int currentInterestPoint = 0;
	GLfloat interestPointTransitionTime = 0.0f;
	GLfloat interestPointTransitionDuration = 2.0f;
	glm::vec3 interestPointStartPos;
	glm::vec3 interestPointStartLookAt;

	CameraMode currentMode = CameraMode::THIRD_PERSON;
	CameraMode pendingMode = CameraMode::THIRD_PERSON;

	// Transición genérica entre modos
	bool inModeTransition = false;
	GLfloat modeTransitionTime = 0.0f;
	GLfloat modeTransitionDuration = 2.0f;
	glm::vec3 transitionStartPos;
	glm::vec3 transitionStartLookAt;
	glm::vec3 transitionTargetPos;
	glm::vec3 transitionTargetLookAt;

	void update();
	void lookAt(const glm::vec3& target);
	// (Ya no está updateModeTransition aquí abajo)
};