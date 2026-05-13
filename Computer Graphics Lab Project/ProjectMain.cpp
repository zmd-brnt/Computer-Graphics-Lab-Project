#include <iostream>
#include <fstream>
#include <cmath>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Header/Animation.h"
#include "Header/Animator.h"

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include "Header/stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Load Models
#include "SOIL2/SOIL2.h"

// Other includes
#include "Header/Shader.h"
#include "Header/Camera.h"
#include "Header/Model.h"
#include "Header/InputManager.h"

// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();

// Window dimensions
const GLuint WIDTH = 1000, HEIGHT = 800;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// ==========================================
// CÁMARA E INPUT MANAGER
// ==========================================
// La altura inicial aquí (2.0f) es solo donde aparece la cámara al abrir la ventana.
Camera camera(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 15.0f, 0.5f);
InputManager inputManager;

GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

bool playFBX = true;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f, 2.0f, 0.0f)
};

// Transformaciones Generales del Modelo
float rotModel = 0.0f;
float modelPosX = 0.0f, modelPosY = 0.0f, modelPosZ = 0.0f;

// Deltatime
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int main()
{
	// Init GLFW
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Animacion Maquina de Estados - Multi Modelos", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

	stbi_set_flip_vertically_on_load(true);

	// ==========================================
	// 1. CARGA DE MODELOS
	// ==========================================
	Model PasilloFI((char*)"Model/PasilloIngenieria.obj");
	Model Vidrios((char*)"Model/Vidrios.obj");
	Model MunecoMadera((char*)"Model/MunecoMadera.fbx");

	Animation animacionMuneco("Model/MunecoMadera.fbx", &MunecoMadera);
	Animator animatorMuneco(&animacionMuneco);

	// ==========================================
	// 2. CONFIGURACIÓN INICIAL DE LA CÁMARA
	// ==========================================
	camera.setCameraMode(CameraMode::THIRD_PERSON);

	// Puntos de interés del pasillo
	// Punto de interés basado en la captura (Vista desde las ventanas amarillas hacia las mesas)
	camera.addInterestPoint(glm::vec3(3.0f, 5.0f, 0.0f), glm::vec3(-1.0f, 1.5f, 0.0f));
	camera.addInterestPoint(glm::vec3(2.0f, 5.0f, -15.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	camera.addInterestPoint(glm::vec3(2.0f, 5.0f, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	camera.addInterestPoint(glm::vec3(0.0f, 1.8f, -6.0f), glm::vec3(0.0f, 1.5f, 0.0f)); 

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		DoMovement();

		if (playFBX)
		{
			animatorMuneco.UpdateAnimation(deltaTime);
		}

		// ==========================================
		// 3. ACTUALIZACIÓN LÓGICA DE LA CÁMARA
		// ==========================================
		if (camera.isInModeTransition()) {
			camera.updateModeTransition(deltaTime);
		}
		else {
			switch (camera.getCameraMode()) {
			case CameraMode::THIRD_PERSON:
				// ¡CORREGIDO! Aquí se aumenta 1.0f extra a la altura de la cámara.
				// modelPosY base + 0.5f (por la altura de dibujo del modelo) + 1.0f (extra solicitado) = 1.5f
				camera.updateThirdPersonCamera(glm::vec3(modelPosX, modelPosY + 1.5f, modelPosZ), rotModel, deltaTime);
				break;
			case CameraMode::AERIAL:
				camera.keyControl(inputManager, deltaTime);
				break;
			case CameraMode::INTEREST_POINT:
				camera.updateInterestPointCamera(deltaTime);
				break;
			}
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		lightingShader.Use();

		// ==========================================
		// 4. PASAR DATOS DE LA CÁMARA A LOS SHADERS
		// ==========================================
		glUniform3f(glGetUniformLocation(lightingShader.Program, "viewPos"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);

		// Directional light
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.3f, 0.3f, 0.3f);

		// Point Light
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.032f);

		// Spotlight
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.getDirection().x, camera.getDirection().y, camera.getDirection().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.7f, 0.7f, 0.7f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(17.5f)));

		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		// Camera matrices
		glm::mat4 view = camera.calculateViewMatrix();
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		// =====================================================
		// DIBUJAR MODELO 1: PASILLO FI
		// =====================================================
		glm::mat4 modelPasillo = glm::mat4(1.0f);
		modelPasillo = glm::translate(modelPasillo, glm::vec3(0.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelPasillo));
		PasilloFI.Draw(lightingShader);


		// =====================================================
		// DIBUJAR MODELO 2: MUÑECO DE MADERA
		// =====================================================
		glm::mat4 modelMuneco = glm::mat4(1.0f);
		modelMuneco = glm::translate(modelMuneco, glm::vec3(modelPosX, 0.5f, modelPosZ));
		modelMuneco = glm::rotate(modelMuneco, glm::radians(rotModel), glm::vec3(0.0f, 1.0f, 0.0f));
		modelMuneco = glm::rotate(modelMuneco, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		modelMuneco = glm::scale(modelMuneco, glm::vec3(0.0015f, 0.0015f, 0.0015f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMuneco));

		std::vector<glm::mat4> transformsMuneco = animatorMuneco.GetFinalBoneMatrices();
		for (int i = 0; i < transformsMuneco.size(); ++i) {
			std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
			GLuint transformLoc = glGetUniformLocation(lightingShader.Program, uniformName.c_str());
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformsMuneco[i]));
		}
		MunecoMadera.Draw(lightingShader);

		// =====================================================
		// DIBUJAR MODELO: VIDRIOS (Transparente)
		// =====================================================
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);

		glm::mat4 modelVidrios = glm::mat4(1.0f);
		modelVidrios = glm::translate(modelVidrios, glm::vec3(0.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelVidrios));
		Vidrios.Draw(lightingShader);

		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);


		// =====================================================
		// DIBUJAR CUBO DE LUZ (Lámpara)
		// =====================================================
		lampShader.Use();
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 modelLamp = glm::mat4(1.0f);
		modelLamp = glm::translate(modelLamp, pointLightPositions[0]);
		modelLamp = glm::scale(modelLamp, glm::vec3(0.2f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelLamp));

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

void DoMovement()
{
	// =====================================
	// Model Controls (Movimiento Relativo)
	// =====================================
	if (camera.getCameraMode() == CameraMode::THIRD_PERSON)
	{
		float speed = 0.01f;   // Velocidad de caminata
		float rotSpeed = 2.0f; // Velocidad de giro

		if (keys[GLFW_KEY_A]) rotModel += rotSpeed;
		if (keys[GLFW_KEY_D]) rotModel -= rotSpeed;

		float rad = glm::radians(rotModel);
		if (keys[GLFW_KEY_W]) {
			modelPosX -= cos(rad) * speed;
			modelPosZ += sin(rad) * speed;
		}
		if (keys[GLFW_KEY_S]) {
			modelPosX += cos(rad) * speed;
			modelPosZ -= sin(rad) * speed;
		}
	}

	// ==========================================
	// CONTROLES DE CAMBIO DE MODO DE CÁMARA
	// ==========================================
	// Tecla 7: Tercera Persona
	if (keys[GLFW_KEY_7]) {
		camera.setCameraMode(CameraMode::THIRD_PERSON);
	}
	// Tecla 8: Modo Aéreo
	if (keys[GLFW_KEY_8]) {
		glm::vec3 targetPos = glm::vec3(modelPosX, 10.0f, modelPosZ);
		glm::vec3 targetLook = targetPos + glm::vec3(0.0f, 0.0f, -10.0f);
		camera.setTransitionTarget(targetPos, targetLook);
		camera.setCameraMode(CameraMode::AERIAL);
	}
	// Tecla 9: Puntos de Interés
	if (keys[GLFW_KEY_9]) {
		camera.setCameraMode(CameraMode::INTEREST_POINT);
	}
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		playFBX = !playFBX;
		printf("Animaciones FBX: %s\n", playFBX ? "REPRODUCIENDO" : "PAUSADAS");
	}

	if (key == GLFW_KEY_N && action == GLFW_PRESS)
	{
		camera.nextInterestPoint();
	}

	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS) keys[key] = true;
		else if (action == GLFW_RELEASE) keys[key] = false;

		inputManager.setKey(key, keys[key]);
	}
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;

	lastX = xPos;
	lastY = yPos;

	camera.mouseControl(xOffset * 0.1f, yOffset * 0.1f);
}