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


// Function prototypes
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();
void AnimacionManual();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;
// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
bool active;

bool playFBX = true;

// Positions of the point lights
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f,2.0f, 0.0f),
	glm::vec3(0.0f,0.0f, 0.0f),
	glm::vec3(0.0f,0.0f,  0.0f),
	glm::vec3(0.0f,0.0f, 0.0f)
};

float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};


glm::vec3 Light1 = glm::vec3(0);
//Anim
float rotBall = 0.0f;
float rotDog = 0.0f;
int dogAnim = 0;
float FLegs = 0.0f;
float RLegs = 0.0f;
float head = 0.0f;
float tail = 0.0f;

//Previo 12 							Calles Cedeño Andros Gael
//28 / 04 / 2026									320004647

//KeyFrames
float dogPosX, dogPosY, dogPosZ;

#define MAX_FRAMES 48
int i_max_steps = 190;
int i_curr_steps = 0;
typedef struct _frame {

	float rotDog;
	float rotDogInc;
	float dogPosX;
	float dogPosY;
	float dogPosZ;
	float incX;
	float incY;
	float incZ;
	//Agregado para animar al perro
	float FLegs;
	float RLegs;
	float head;
	float tail;
	float FLegsInc;
	float RLegsInc;
	float headInc;
	float tailInc;

}FRAME;
//Previo 12 							Calles Cedeño Andros Gael
//28 / 04 / 2026									320004647
FRAME KeyFrame[MAX_FRAMES];
//Momento en la linea del tiempo en donde inicia
int FrameIndex = 0;			//introducir datos
bool play = false;
//En que punto en la linea se encuentra
int playIndex = 0;

//Funcion para guardar los KeyFrames
void saveFrame(void)
{

	printf("frameindex %d\n", FrameIndex);

	//Guardar el estado actual de las variables del modelo, guardandolo a la estructura _frame dentro del elemento correspondiente
	KeyFrame[FrameIndex].dogPosX = dogPosX;
	KeyFrame[FrameIndex].dogPosY = dogPosY;
	KeyFrame[FrameIndex].dogPosZ = dogPosZ;

	KeyFrame[FrameIndex].rotDog = rotDog;
	//Implelmentacion para el cuerpo completo del perro
	KeyFrame[FrameIndex].FLegs = FLegs;
	KeyFrame[FrameIndex].RLegs = RLegs;
	KeyFrame[FrameIndex].head = head;
	KeyFrame[FrameIndex].tail = tail;
	//Y luego incrementamos el frame para el siguiente a guardar
	FrameIndex++;
}

//Regresa a la posicion inicial 
void resetElements(void)
{
	dogPosX = KeyFrame[0].dogPosX;
	dogPosY = KeyFrame[0].dogPosY;
	dogPosZ = KeyFrame[0].dogPosZ;

	rotDog = KeyFrame[0].rotDog;
	FLegs = KeyFrame[0].FLegs;
	RLegs = KeyFrame[0].RLegs;
	head = KeyFrame[0].head;
	tail = KeyFrame[0].tail;


}

//Realiza la interpolacion entre un keyframe a otro 
void interpolation(void)
{
	//Calculamos el incremento entre keyframes dandonos la posicion del frame actual al siguiente, para guardar la informacion en la variable correspondiente y 
	// le restamos la posicion y rotacion del frame actual
	KeyFrame[playIndex].incX = (KeyFrame[playIndex + 1].dogPosX - KeyFrame[playIndex].dogPosX) / i_max_steps;
	KeyFrame[playIndex].incY = (KeyFrame[playIndex + 1].dogPosY - KeyFrame[playIndex].dogPosY) / i_max_steps;
	KeyFrame[playIndex].incZ = (KeyFrame[playIndex + 1].dogPosZ - KeyFrame[playIndex].dogPosZ) / i_max_steps;

	KeyFrame[playIndex].rotDogInc = (KeyFrame[playIndex + 1].rotDog - KeyFrame[playIndex].rotDog) / i_max_steps;
	KeyFrame[playIndex].FLegsInc = (KeyFrame[playIndex + 1].FLegs - KeyFrame[playIndex].FLegs) / i_max_steps;
	KeyFrame[playIndex].RLegsInc = (KeyFrame[playIndex + 1].RLegs - KeyFrame[playIndex].RLegs) / i_max_steps;
	KeyFrame[playIndex].headInc = (KeyFrame[playIndex + 1].head - KeyFrame[playIndex].head) / i_max_steps;
	KeyFrame[playIndex].tailInc = (KeyFrame[playIndex + 1].tail - KeyFrame[playIndex].tail) / i_max_steps;
}


//Implementacion de sistema de guardado a partir de la libreria fstream
//para crear archivos txt con los keyframes y pueda leerlos a posteridad 

	//Practica 12 							Calles Cedeño Andros Gael
	//03 / 05 / 2026									320004647
// Guarda los keyframes en un formato "dosPosX,dogPosY, dogPosZ, rotDog, FLegs, RLegs,head,tail" y luego salta de linea para guardar el siguiente keyframe
void saveKeyframesToFile(const std::string& filename) {
	std::ofstream file(filename);
	if (file.is_open()) {
		// Guardamos cuántos frames hay actualmente
		file << FrameIndex << "\n";

		for (int i = 0; i < FrameIndex; i++) {
			file << KeyFrame[i].dogPosX << " "
				<< KeyFrame[i].dogPosY << " "
				<< KeyFrame[i].dogPosZ << " "
				<< KeyFrame[i].rotDog << " "
				<< KeyFrame[i].FLegs << " "
				<< KeyFrame[i].RLegs << " "
				<< KeyFrame[i].head << " "
				<< KeyFrame[i].tail << "\n";
		}
		// Cuando se acabe de guardar y se presiona la tecla O se graban todos los keyframes en un txt
		file.close();
		printf("Keyframes guardados en %s\n", filename.c_str());
	}
	else {
		printf("Error al abrir el archivo para guardar.\n");
	}
}
//Practica 12 							Calles Cedeño Andros Gael
//03 / 05 / 2026									320004647
//Lee el archivo TXT asegurandose de que tenga el formato anteriormente dicho
void loadKeyframesFromFile(const std::string& filename) {
	std::ifstream file(filename);
	if (file.is_open()) {
		file >> FrameIndex; // Leemos el número de frames guardados

		for (int i = 0; i < FrameIndex; i++) {
			file >> KeyFrame[i].dogPosX
				>> KeyFrame[i].dogPosY
				>> KeyFrame[i].dogPosZ
				>> KeyFrame[i].rotDog
				>> KeyFrame[i].FLegs
				>> KeyFrame[i].RLegs
				>> KeyFrame[i].head
				>> KeyFrame[i].tail;

			// Limpiamos los incrementos para evitar basura de sesiones anteriores
			KeyFrame[i].incX = 0; KeyFrame[i].incY = 0; KeyFrame[i].incZ = 0;
			KeyFrame[i].rotDogInc = 0; KeyFrame[i].FLegsInc = 0;
			KeyFrame[i].RLegsInc = 0; KeyFrame[i].headInc = 0; KeyFrame[i].tailInc = 0;
		}
		file.close();
		printf("Keyframes cargados desde %s. Total: %d\n", filename.c_str(), FrameIndex);

		// Resetear el modelo a la posición del primer frame cargado
		if (FrameIndex > 0) resetElements();
	}
	else {
		printf("No se encontro el archivo de guardado.\n");
	}
}

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);*/

	// Create a GLFWwindow object that we can use for GLFW's functions

	//Practica 12 							Calles Cedeño Andros Gael
	//03 / 05 / 2026									320004647

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Animacion maquina de estados_Calles Cedeño Andros Gael", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();

		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Set the required callback functions
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// GLFW Options
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (GLEW_OK != glewInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define the viewport dimensions
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);



	Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
	Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");


	//models
	// 
	// Aplica las transformaciones en Blender: Antes de exportar a FBX, selecciona tu modelo y tu esqueleto en Blender. Presiona Ctrl + A y selecciona "All Transforms" (Todas las transformaciones). Esto asegura que la escala esté en 1.0 y la rotación en 0 para evitar comportamientos extraños.

	// Verifica los pesos(Weight Paint) : Asegúrate de que todos los vértices de tu malla tengan algún peso asignado a algún hueso.Si un vértice no tiene peso, su posición final será(0, 0, 0), lo que causa que la malla colapse hacia el centro(que parece ser en parte lo que está ocurriendo en tu imagen).
	//Configuración de exportación FBX :

	//En "Transform", asegúrate de que "Apply Scalings" esté en "FBX All" o "FBX Units Scale".

	//	En "Geometry", marca "Apply Modifiers".

	//	En "Armature", desmarca "Add Leaf Bones" (suele generar huesos extra innecesarios).
	Model MuñecoMadera((char*)"Models/MuñecoMaderaRIGGEADO3.fbx");
	Model Piso((char*)"Models/Piso.obj");

	// 1. Instanciamos la animación asegurándonos del punto y coma al final
	Animation animacionMuñeco("Models/MuñecoMaderaRIGGEADO3.fbx", &MuñecoMadera);

	// 2. Iniciamos el animador con esa animación en una línea nueva
	Animator animator(&animacionMuñeco);

	//KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].dogPosX = 0;
		KeyFrame[i].dogPosY = 0;
		KeyFrame[i].dogPosZ = 0;
		KeyFrame[i].incX = 0;
		KeyFrame[i].incY = 0;
		KeyFrame[i].incZ = 0;
		KeyFrame[i].rotDog = 0;
		KeyFrame[i].rotDogInc = 0;
		KeyFrame[i].FLegs = 0;
		KeyFrame[i].RLegs = 0;
		KeyFrame[i].head = 0;
		KeyFrame[i].tail = 0;
		KeyFrame[i].FLegsInc = 0;
		KeyFrame[i].RLegsInc = 0;
		KeyFrame[i].headInc = 0;
		KeyFrame[i].tailInc = 0;
	}


	// First, set the container's VAO (and VBO)
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);


	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Set texture units
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);


	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);




	// Game loop
	while (!glfwWindowShouldClose(window))
	{

		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		DoMovement();
		AnimacionManual();
		
		if (playFBX)
		{
			// Solo avanzamos el tiempo si la bandera es verdadera
			animator.UpdateAnimation(deltaTime);
		}

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// OpenGL options
		glEnable(GL_DEPTH_TEST);


		glm::mat4 modelTemp = glm::mat4(1.0f); //Temp



		// Use cooresponding shader when setting uniforms/drawing objects
		lightingShader.Use();

		glUniform1i(glGetUniformLocation(lightingShader.Program, "diffuse"), 0);
		//glUniform1i(glGetUniformLocation(lightingShader.Program, "specular"),1);

		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);


		// Directional light
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.6f, 0.6f, 0.6f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.3f, 0.3f, 0.3f);


		// Point light 1
		glm::vec3 lightColor;
		lightColor.x = abs(sin(glfwGetTime() * Light1.x));
		lightColor.y = abs(sin(glfwGetTime() * Light1.y));
		lightColor.z = sin(glfwGetTime() * Light1.z);


		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 0.2f, 0.2f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.045f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.075f);


		// SpotLight
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), camera.GetFront().x, camera.GetFront().y, camera.GetFront().z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), 0.2f, 0.2f, 0.8f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.3f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.7f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(12.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(18.0f)));


		// Set material properties
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 5.0f);

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Get the uniform locations
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		glm::mat4 model(1);



		//Carga de modelo 
		view = camera.GetViewMatrix();
		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Piso.Draw(lightingShader);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(dogPosX, dogPosY, dogPosZ));

		// NOTA: Agregué un "-90.0f" aquí para contrarrestar que el FBX esté acostado
		model = glm::rotate(model, glm::radians( rotDog), glm::vec3(1.0f, 0.0f, 0.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);

		// --- EL CÓDIGO CRÍTICO PARA LA ANIMACIÓN ---
		// Obtenemos las transformaciones calculadas de los huesos en ESTE frame
		std::vector<glm::mat4> transforms = animator.GetFinalBoneMatrices();

		// Las enviamos al arreglo "finalBonesMatrices" de tu Vertex Shader
		for (int i = 0; i < transforms.size(); ++i) {
			std::string uniformName = "finalBonesMatrices[" + std::to_string(i) + "]";
			GLuint transformLoc = glGetUniformLocation(lightingShader.Program, uniformName.c_str());
			glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transforms[i]));
		}
		// -------------------------------------------

		// 4. Finalmente, dibujamos la malla YA CON LAS MATRICES CARGADAS
		MuñecoMadera.Draw(lightingShader);
		glBindVertexArray(0);

		// Also draw the lamp object, again binding the appropriate shader
		lampShader.Use();
		// Get location objects for the matrices on the lamp shader (these could be different on a different shader)
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Set matrices
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		// Draw the light object (using light's vertex attributes)

		model = glm::mat4(1);
		model = glm::translate(model, pointLightPositions[0]);
		model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);


		// Swap the screen buffers
		glfwSwapBuffers(window);
	}




	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();



	return 0;
}

// Moves/alters the camera positions based on user input
void DoMovement()
{
	//Dog Controls

	if (keys[GLFW_KEY_2])
	{

		rotDog += 1.0f;

	}

	if (keys[GLFW_KEY_3])
	{

		rotDog -= 1.0f;

	}

	if (keys[GLFW_KEY_4])
	{


		FLegs -= 1.0f;

	}
	if (keys[GLFW_KEY_5])
	{

		FLegs += 1.0f;

	}
	if (keys[GLFW_KEY_6])
	{

		RLegs -= 1.0f;

	}
	if (keys[GLFW_KEY_7])
	{

		RLegs += 1.0f;

	}
	if (keys[GLFW_KEY_8])
	{

		head -= 1.0f;

	}
	if (keys[GLFW_KEY_9])
	{

		head += 1.0f;

	}

	if (keys[GLFW_KEY_Z])
	{

		tail -= 1.0f;

	}

	if (keys[GLFW_KEY_X])
	{

		tail += 1.0f;

	}

	if (keys[GLFW_KEY_H])
	{
		dogPosZ += 0.01;
	}

	if (keys[GLFW_KEY_Y])
	{
		dogPosZ -= 0.01;
	}

	if (keys[GLFW_KEY_G])
	{
		dogPosX -= 0.01;
	}

	if (keys[GLFW_KEY_J])
	{
		dogPosX += 0.01;
	}
	if (keys[GLFW_KEY_F])
	{
		dogPosY -= 0.01;
	}

	if (keys[GLFW_KEY_R])
	{
		dogPosY += 0.01;
	}

	// Camera controls
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);

	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);


	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime);


	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);


	}

	if (keys[GLFW_KEY_T])
	{
		pointLightPositions[0].x += 0.01f;
	}
	if (keys[GLFW_KEY_G])
	{
		pointLightPositions[0].x -= 0.01f;
	}

	if (keys[GLFW_KEY_Y])
	{
		pointLightPositions[0].y += 0.01f;
	}

	if (keys[GLFW_KEY_H])
	{
		pointLightPositions[0].y -= 0.01f;
	}
	if (keys[GLFW_KEY_U])
	{
		pointLightPositions[0].z -= 0.1f;
	}
	if (keys[GLFW_KEY_J])
	{
		pointLightPositions[0].z += 0.01f;
	}

}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		playFBX = !playFBX; // Alterna entre true y false
		printf("Animacion FBX: %s\n", playFBX ? "REPRODUCIENDO" : "PAUSADA");
	}
	//Practica 12 							Calles Cedeño Andros Gael
	//03 / 05 / 2026									320004647
	// Guardar con la Tecla P los frames en un txt
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		saveKeyframesToFile("animacion_perro.txt");
	}

	//Para reproducir las animaciones
	if (keys[GLFW_KEY_L])
	{
		//Si play es falso y tenemos keyframes 
		//Se inicia el frame 0 y se realiza la interpolacion
		if (play == false && (FrameIndex > 1))
		{

			resetElements();
			//First Interpolation				
			interpolation();
			// 
			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
		}

	}
	// Cargar con la Tecla O la animacion que se guardo con la Tecla P
	if (key == GLFW_KEY_O && action == GLFW_PRESS)
	{
		loadKeyframesFromFile("animacion_perro.txt");
	}


	//Para guardar un frame
	if (keys[GLFW_KEY_K])
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}

	}



	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_SPACE])
	{
		active = !active;
		if (active)
		{
			Light1 = glm::vec3(0.2f, 0.8f, 1.0f);

		}
		else
		{
			Light1 = glm::vec3(0);//Cuado es solo un valor en los 3 vectores pueden dejar solo una componente
		}
	}


}
void AnimacionManual() {

	//Si ejecutamos play
	if (play)
	{
		//Mientras no lleguemos al final de la animacion
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			//Incrementamos cada inidice
			playIndex++;
			//Mientras este en el rango

			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
				//Interpolation
				interpolation();
			}
		}
		else
		{
			//Previo 12 							Calles Cedeño Andros Gael
			//28 / 04 / 2026									320004647
			//Draw animation
			dogPosX += KeyFrame[playIndex].incX;
			dogPosY += KeyFrame[playIndex].incY;
			dogPosZ += KeyFrame[playIndex].incZ;
			tail += KeyFrame[playIndex].tailInc;
			RLegs += KeyFrame[playIndex].RLegsInc;
			FLegs += KeyFrame[playIndex].FLegsInc;
			head += KeyFrame[playIndex].headInc;
			rotDog += KeyFrame[playIndex].rotDogInc;

			i_curr_steps++;
		}

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
	GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}