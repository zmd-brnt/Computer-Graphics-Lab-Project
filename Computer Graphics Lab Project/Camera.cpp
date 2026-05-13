#include "Header/Camera.h"
#include <algorithm>

Camera::Camera()
	: position(0.0f), front(0.0f, 0.0f, -1.0f), up(0.0f),
	right(0.0f), worldUp(0.0f, 1.0f, 0.0f),
	yaw(-90.0f), pitch(0.0f), moveSpeed(0.0f), turnSpeed(0.0f)
{
}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp,
	GLfloat startYaw, GLfloat startPitch,
	GLfloat startMoveSpeed, GLfloat turnSpeed)
	: position(startPosition), worldUp(startUp),
	yaw(startYaw), pitch(startPitch),
	front(0.0f, 0.0f, -1.0f),
	moveSpeed(startMoveSpeed), turnSpeed(turnSpeed),
	aerialPosition(startPosition)
{
	update();
}

// Helpers
void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

void Camera::lookAt(const glm::vec3& target)
{
	front = glm::normalize(target - position);

	// --- FIX DE PANTALLA NEGRA ---
	// Si la cámara mira casi exactamente hacia abajo o hacia arriba, 
	// usamos un vector temporal distinto para evitar el producto cruz = 0 (NaN).
	if (std::abs(front.y) > 0.999f) {
		right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 0.0f, -1.0f)));
	}
	else {
		right = glm::normalize(glm::cross(front, worldUp));
	}
	// -----------------------------

	up = glm::normalize(glm::cross(right, front));

	yaw = glm::degrees(atan2(front.z, front.x));
	pitch = glm::degrees(asin(glm::clamp(front.y, -1.0f, 1.0f)));
}

// Transición suave entre modos
bool Camera::updateModeTransition(GLfloat deltaTime)
{
	if (!inModeTransition) return false;

	modeTransitionTime += deltaTime;
	GLfloat t = std::min(modeTransitionTime / modeTransitionDuration, 1.0f);
	// Ease in-out cúbico: suaviza arranque y frenado
	GLfloat tSmooth = t * t * (3.0f - 2.0f * t);

	position = glm::mix(transitionStartPos, transitionTargetPos, tSmooth);
	glm::vec3 currentLookAt = glm::mix(transitionStartLookAt, transitionTargetLookAt, tSmooth);
	lookAt(currentLookAt);

	if (t >= 1.0f)
	{
		inModeTransition = false;
		currentMode = pendingMode;

		// Al llegar al modo aéreo, sincronizar aerialPosition con la posición final
		if (currentMode == CameraMode::AERIAL)
			aerialPosition = position;
	}

	return true;
}

// Cambiar el modo de cámara
void Camera::setCameraMode(CameraMode mode)
{
	if (mode == currentMode && !inModeTransition) return;

	// Completar transición genérica pendiente antes de iniciar otra.
	if (inModeTransition)
	{
		inModeTransition = false;
		currentMode = pendingMode;
		if (currentMode == CameraMode::AERIAL)
			aerialPosition = position;
	}

	// Entrada a puntos de interés
	if (mode == CameraMode::INTEREST_POINT)
	{
		currentMode = mode;
		if (!interestPoints.empty())
		{
			interestPointTransitionTime = 0.0f;
			currentInterestPoint = 0;
			interestPointStartPos = position;
			interestPointStartLookAt = position + front;
		}
		return;
	}

	pendingMode = mode;
	inModeTransition = true;
	modeTransitionTime = 0.0f;
	transitionStartPos = position;
	transitionStartLookAt = position + front;
}

// Control de la transición
void Camera::setTransitionTarget(const glm::vec3& targetPos, const glm::vec3& targetLookAt)
{
	transitionTargetPos = targetPos;
	transitionTargetLookAt = targetLookAt;
}

// Input
void Camera::keyControl(const InputManager& input, GLfloat deltaTime)
{
	if (currentMode == CameraMode::THIRD_PERSON) return;
	if (currentMode == CameraMode::AERIAL)
	{
		updateAerialCamera(input, deltaTime);
		return;
	}
	// Fallback primera persona
	GLfloat velocity = moveSpeed * deltaTime;
	if (input.isKeyDown(GLFW_KEY_W)) position += front * velocity;
	if (input.isKeyDown(GLFW_KEY_S)) position -= front * velocity;
	if (input.isKeyDown(GLFW_KEY_A)) position -= right * velocity;
	if (input.isKeyDown(GLFW_KEY_D)) position += right * velocity;
}

void Camera::mouseControl(GLfloat xDelta, GLfloat yDelta)
{
	if (currentMode == CameraMode::INTEREST_POINT) return;

	yaw += xDelta;
	pitch -= yDelta;
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;
	update();
}

// Matriz de vista
glm::mat4 Camera::calculateViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

// Modo de tercera persona
void Camera::updateThirdPersonCamera(const glm::vec3& targetPosition, GLfloat modelRotation, GLfloat deltaTime)
{
	if (currentMode != CameraMode::THIRD_PERSON) return;

	// 1. Convertimos la rotación del modelo de grados a radianes
	float rad = glm::radians(modelRotation);

	// 2. Calculamos hacia dónde mira el modelo. 
	// (En tu código original, W restaba en el eje X, por lo que su frente natural es -X)
	glm::vec3 modelForward(-cos(rad), 0.0f, sin(rad));

	// 3. Calculamos la posición deseada de la cámara: 
	// Estará en la dirección OPUESTA al frente (-modelForward) multiplicada por la distancia
	glm::vec3 offset = -modelForward * thirdPersonDistance;
	glm::vec3 desiredPos = targetPosition + offset;
	desiredPos.y = targetPosition.y + thirdPersonHeight;

	// 4. Movemos la cámara suavemente hacia esa posición
	position = glm::mix(position, desiredPos, smoothSpeed * deltaTime);

	// 5. Miramos hacia la cabeza/espalda del muñeco
	glm::vec3 lookTarget = targetPosition;
	lookTarget.y += 0.8f;
	lookAt(lookTarget);
}

// Modo aéreo
void Camera::updateAerialCamera(const InputManager& input, GLfloat deltaTime)
{
	if (currentMode != CameraMode::AERIAL) return;

	GLfloat velocity = aerialMoveSpeed * deltaTime;

	if (input.isKeyDown(GLFW_KEY_W)) aerialPosition.z -= velocity;
	if (input.isKeyDown(GLFW_KEY_S)) aerialPosition.z += velocity;
	if (input.isKeyDown(GLFW_KEY_A)) aerialPosition.x -= velocity;
	if (input.isKeyDown(GLFW_KEY_D)) aerialPosition.x += velocity;
	if (input.isKeyDown(GLFW_KEY_Q)) aerialPosition.y += velocity;
	if (input.isKeyDown(GLFW_KEY_E)) aerialPosition.y -= velocity;

	aerialPosition.y = glm::clamp(aerialPosition.y, 5.0f, 50.0f);

	const float MAP_LIMIT = 70.0f;
	aerialPosition.x = glm::clamp(aerialPosition.x, -MAP_LIMIT, MAP_LIMIT);
	aerialPosition.z = glm::clamp(aerialPosition.z, -MAP_LIMIT, MAP_LIMIT);

	position = aerialPosition;

	glm::vec3 dirToCenter = glm::normalize(glm::vec3(0.0f) - aerialPosition);
	lookAt(aerialPosition + dirToCenter * 30.0f);
	pitch = -45.0f;
	update();
}

// Modo de puntos de interés
void Camera::addInterestPoint(const glm::vec3& point, const glm::vec3& lookAtPoint)
{
	InterestPoint ip;
	ip.cameraPosition = point;
	ip.lookAtPoint = lookAtPoint;
	interestPoints.push_back(ip);
}

void Camera::nextInterestPoint()
{
	if (interestPoints.empty()) return;

	interestPointTransitionTime = 0.0f;
	interestPointStartPos = position;
	interestPointStartLookAt = position + front;
	currentInterestPoint = (currentInterestPoint + 1) % interestPoints.size();
}

void Camera::updateInterestPointCamera(GLfloat deltaTime)
{
	if (updateModeTransition(deltaTime)) return;

	if (currentMode != CameraMode::INTEREST_POINT || interestPoints.empty()) return;

	interestPointTransitionTime += deltaTime;
	GLfloat t = std::min(interestPointTransitionTime / interestPointTransitionDuration, 1.0f);
	GLfloat tSmooth = t * t * (3.0f - 2.0f * t);

	const InterestPoint& ip = interestPoints[currentInterestPoint];
	position = glm::mix(interestPointStartPos, ip.cameraPosition, tSmooth);
	glm::vec3 targetLookAt = glm::mix(interestPointStartLookAt, ip.lookAtPoint, tSmooth);
	lookAt(targetLookAt);
}

Camera::~Camera() {}