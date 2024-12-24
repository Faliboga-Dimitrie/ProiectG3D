#include <Windows.h>
#include <locale>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ShaderManager.h"
#include "Model.h"
#include "Camera.h"
#include "SkyBox.h"
#include "Pilot.h"

//#include "CameraMovementType.h"

// settings

Camera* pCamera = nullptr;
SkyBox* skybox = nullptr;

void Cleanup()
{
	delete pCamera;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void RenderScene(ShaderManager& shaderManager, std::unordered_map<std::string,Model> models);
void RenderSkybox(Shader& shader);
void RenderTrack(Shader& shader, Model& model);
void RenderKart(Shader& shader, Model& KartModel, Model& PilotModel);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

glm::vec3 kartPos(0.0f, 0.0f, 0.0f);  // Poziția inițială a kart-ului
float kartSpeed = 5.0f;               // Viteza de mișcare a kart-ului


int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	ShaderManager shaderManager;
	std::unordered_map<std::string, Model> models;

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Lab 7", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glViewport(0, 0, mode->width, mode->height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	glEnable(GL_DEPTH_TEST);

	/*float vertices[] = {
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
	unsigned int VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);*/

	pCamera = new Camera(mode->width, mode->height, glm::vec3(0.0, 2.0, 6.0));

	glm::vec3 lightPos(0.0f, 2.0f, 1.0f);
	glm::vec3 cubePos(0.0f, 5.0f, 1.0f);

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);

	SkyBox* skybox = new SkyBox(currentPath + "\\Textures\\SkyBox\\", "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg");

	shaderManager.LoadShader("lightingShader", currentPath + "\\Shaders\\PhongLight.vs", currentPath + "\\Shaders\\PhongLight.fs");
	shaderManager.LoadShader("lightingWithTextureShader", currentPath + "\\Shaders\\PhongLightWithTexture.vs", currentPath + "\\Shaders\\PhongLightWithTexture.fs");
	shaderManager.LoadShader("lampShader", currentPath + "\\Shaders\\Lamp.vs", currentPath + "\\Shaders\\Lamp.fs");
	shaderManager.LoadShader("skyboxShader", currentPath + "\\Shaders\\SkyBox.vs", currentPath + "\\Shaders\\SkyBox.fs");

	std::string go_kartObjFileName = (currentPath + "\\Models\\Kart\\go_kart.obj");
	Model go_kartObjModel(go_kartObjFileName, false);
	models["go_kart"] = go_kartObjModel;

	std::string pilotObjFileName = (currentPath + "\\Models\\Pilot\\pilot.obj");
	Pilot PilotModel(pilotObjFileName, false);
	models["pilot"] = PilotModel;

	std::string TrackObjFileName = (currentPath + "\\Models\\Track\\track.obj");
	Model TrackModel(TrackObjFileName, false);
	models["track"] = TrackModel;

	while (!glfwWindowShouldClose(window)) 
	{
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);

		lightPos.x = 5.0 * cos(glfwGetTime());
		lightPos.z = 5.0 * sin(glfwGetTime());
		cubePos.x = 10 * sin(glfwGetTime());
		cubePos.z = 10 * cos(glfwGetTime());

		//Skybox
		Shader skyboxShader = shaderManager.GetShader("skyboxShader");
		skyboxShader.use();
		skyboxShader.setMat4("projection", pCamera->GetProjectionMatrix());
		skyboxShader.setMat4("view", glm::mat4(glm::mat3(pCamera->GetViewMatrix())));
		skybox->Render();

		Shader lightingShader = shaderManager.GetShader("lightingShader");
		lightingShader.use();
		lightingShader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
		lightingShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.SetVec3("lightPos", lightPos);
		lightingShader.SetVec3("viewPos", pCamera->GetPosition());

		lightingShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingShader.setMat4("view", pCamera->GetViewMatrix());

		// render the model
		Shader lightingWithTextureShader = shaderManager.GetShader("lightingWithTextureShader");
		lightingWithTextureShader.use();
		lightingWithTextureShader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
		lightingWithTextureShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingWithTextureShader.SetVec3("lightPos", lightPos);
		lightingWithTextureShader.SetVec3("viewPos", pCamera->GetPosition());
		lightingWithTextureShader.setInt("texture_diffuse1", 0);

		lightingWithTextureShader.setMat4("projection", pCamera->GetProjectionMatrix());
		lightingWithTextureShader.setMat4("view", pCamera->GetViewMatrix());

		//go_kart model
		glm::mat4 go_kartModel = glm::mat4(1.0f);
		go_kartModel = glm::translate(go_kartModel, kartPos);       // Aplică poziția kart-ului
		go_kartModel = glm::scale(go_kartModel, glm::vec3(0.06f));  // Aplică scala kart-ului
		glm::vec3 kartForwardVector = glm::normalize(glm::vec3(go_kartModel[2]));  // Obține vectorul de direcție al kart-ului
		pCamera->UpdateKartPosition(kartPos, kartForwardVector);


		PilotModel.UpdatePosition(kartPos);
		lightingWithTextureShader.use();
		lightingWithTextureShader.setMat4("model", go_kartModel);
		go_kartObjModel.Draw(lightingWithTextureShader);
		PilotModel.Draw(lightingWithTextureShader);


		glm::mat4 pilotModel = glm::mat4(1.0f);
		pilotModel = glm::scale(pilotModel, glm::vec3(0.02f)); // Ajustează scala pilotului
		pilotModel = glm::translate(pilotModel, glm::vec3(0.0f, 1.2f, 0.0f)); // Poziționează pilotul în kart
		pilotModel = go_kartModel * pilotModel; // Aplică transformările kart-ului la pilot
		lightingWithTextureShader.setMat4("model", pilotModel);

		//track model
		glm::mat4 trackModel = glm::mat4(1.0f);
		trackModel = glm::scale(trackModel, glm::vec3(200.0f)); // Ajustează scala pistei
		trackModel = glm::translate(trackModel, glm::vec3(0.0f, -0.008f, 0.0f)); // Poziționează pista
		lightingWithTextureShader.setMat4("model", trackModel);
		TrackModel.Draw(lightingWithTextureShader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();

	/*glDeleteVertexArrays(1, &cubeVAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);*/

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}

void processInput(GLFWwindow* window) 
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		pCamera->ProcessKeyboard(CameraMovementType::DOWN, (float)deltaTime);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		kartPos.z += kartSpeed * (float)deltaTime;  // Mișcare înainte

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		kartPos.z -= kartSpeed * (float)deltaTime;  // Mișcare înapoi

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		kartPos.x += kartSpeed * (float)deltaTime;  // Mișcare la stânga

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		kartPos.x -= kartSpeed * (float)deltaTime;  // Mișcare la dreapta

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		pCamera->SetCameraMode(CameraMode::FREE);  // Mișcare libera a camerei

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		pCamera->SetCameraMode(CameraMode::FIRST_PERSON);  // Modul first person

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		pCamera->SetCameraMode(CameraMode::THIRD_PERSON);  // Modul third person

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);
	}
}

void RenderScene(ShaderManager& shaderManager, std::unordered_map<std::string, Model> models) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RenderSkybox(shaderManager.GetShader("skyboxShader"));
	RenderKart(shaderManager.GetShader("lightingWithTextureShader"),models.at("go_kart"),models.at("pilot"));
	RenderTrack(shaderManager.GetShader("lightingWithTextureShader"), models.at("track"));
}

void RenderSkybox(Shader& shader)
{
	shader.use();
	shader.setMat4("projection", pCamera->GetProjectionMatrix());
	shader.setMat4("view", glm::mat4(glm::mat3(pCamera->GetViewMatrix())));
	skybox->Render();
}

void RenderKart(Shader& shader, Model& KartModel, Model& PilotModel)
{
	glm::mat4 go_kartModel = glm::mat4(1.0f);
	go_kartModel = glm::translate(go_kartModel, kartPos);
	go_kartModel = glm::scale(go_kartModel, glm::vec3(0.06f));

	shader.use();
	shader.setMat4("model", go_kartModel);
	KartModel.Draw(shader);

	glm::mat4 pilotModel = glm::mat4(1.0f);
	pilotModel = glm::scale(pilotModel, glm::vec3(0.02f));
	pilotModel = glm::translate(pilotModel, glm::vec3(0.0f, 1.2f, 0.0f));
	pilotModel = go_kartModel * pilotModel;

	shader.setMat4("model", pilotModel);
	PilotModel.Draw(shader);
}

void RenderTrack(Shader& shader, Model& model)
{
	glm::mat4 trackModel = glm::mat4(1.0f);
	trackModel = glm::scale(trackModel, glm::vec3(0.02f));
	trackModel = glm::translate(trackModel, glm::vec3(0.0f, -600.0f, 0.0f));

	shader.use();
	shader.setMat4("model", trackModel);
	model.Draw(shader);
}

