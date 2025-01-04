#include <Windows.h>
#include <locale>
#include <codecvt>
#include <iostream>
#include <fstream>
#include <sstream>
#include <xmemory>

#include "ShaderManager.h"
#include "Model.h"
#include "Camera.h"
#include "SkyBox.h"
#include "Pilot.h"

//#include "CameraMovementType.h"

// settings

std::shared_ptr<Camera> pCamera = nullptr;
std::shared_ptr<SkyBox> skybox = nullptr;

struct Kart {
	glm::vec3 position;
	glm::vec3 kartDirection;
	float rotationAngle;
	float kartAcceleration;
	Model kartModel;
	std::shared_ptr<Pilot> pilotModel;
	bool isCurrentPlayer = false;
	bool kartAccelerationChanged = false;

	Kart() = default;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void RenderScene(ShaderManager& shaderManager, std::unordered_map<std::string,Model>& models, std::vector<Kart>& karts, std::shared_ptr<SkyBox> skyBoxInstance);
void RenderSkybox(Shader& shader,std::shared_ptr<SkyBox> skyBoxInstance);
void RenderTrack(Shader& shader, Model& model);
void RenderKarts(Shader& shader, std::vector<Kart>& karts);
void RenderTerrain(Shader& shader, Model& model);
void LoadMultipleKarts(std::vector<Kart>& karts, std::unordered_map<std::string, Model>& models, std::shared_ptr<Pilot> pilot);
bool checkCollision(const Kart& kart1, const Kart& kart2);
void handleCollision(Kart& kart1, Kart& kart2);

// timing
double deltaTime = 0.0f;	// time between current frame and last frame
double lastFrame = 0.0f;

glm::vec3 kartPos(39.0f, 0.0f, 320.0f);  // Poziția inițială a kart-ului
float kartSpeed = 15.0f;               // Viteza de mișcare a kart-ului

glm::vec3 lightPos(0.0f, 2.0f, 1.0f); // Pozitia luminii

float kartRotationAngle = 0.0f; // Unghiul curent al kart-ului
float kartRotationSpeed = 90.0f; // Viteza de rotație în grade pe secundă

float kartAcceleration = 0.0f; // Accelerarea curentă
float kartMaxSpeed = 50.0f; // Viteza maximă
float kartAccelerationRate = 20.0f; // Rata de accelerare
float kartDecelerationRate = 5.0f; // Rata de decelerare

int currentKartIndex = 5; // Indexul următorului kart
int lastKartIndex = -1; // Indexul kart-ului anterior

std::vector<Kart> karts;

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

	pCamera = std::make_shared<Camera>(mode->width, mode->height, glm::vec3(0.0, 2.0, 6.0));

	glm::vec3 cubePos(0.0f, 5.0f, 1.0f);

	wchar_t buffer[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, buffer);

	std::wstring executablePath(buffer);
	std::wstring wscurrentPath = executablePath.substr(0, executablePath.find_last_of(L"\\/"));

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string currentPath = converter.to_bytes(wscurrentPath);

	skybox = std::make_shared<SkyBox>(currentPath + "\\Textures\\SkyBox\\", "back.jpg", "front.jpg", "top.jpg", "bottom.jpg", "left.jpg", "right.jpg");

	shaderManager.LoadShader("lightingShader", currentPath + "\\Shaders\\PhongLight.vs", currentPath + "\\Shaders\\PhongLight.fs");
	shaderManager.LoadShader("lightingWithTextureShader", currentPath + "\\Shaders\\PhongLightWithTexture.vs", currentPath + "\\Shaders\\PhongLightWithTexture.fs");
	shaderManager.LoadShader("lampShader", currentPath + "\\Shaders\\Lamp.vs", currentPath + "\\Shaders\\Lamp.fs");
	shaderManager.LoadShader("skyboxShader", currentPath + "\\Shaders\\SkyBox.vs", currentPath + "\\Shaders\\SkyBox.fs");
	shaderManager.LoadShader("shadowMappingDepthShader", currentPath + "\\Shaders\\ShadowMappingDepth.vs", currentPath + "\\Shaders\\ShadowMappingDepth.fs");

	std::string go_kartObjFileName = (currentPath + "\\Models\\Kart\\go_kart.obj");
	Model go_kartObjModel(go_kartObjFileName, false);
	models["go_kart"] = go_kartObjModel;

	std::string pilotObjFileName = (currentPath + "\\Models\\Pilot\\pilot.obj");
	std::shared_ptr<Pilot> PilotModel = std::make_shared<Pilot>(pilotObjFileName, false);

	LoadMultipleKarts(karts, models, PilotModel);

	std::string TrackObjFileName = (currentPath + "\\Models\\Track\\track.obj");
	Model TrackModel(TrackObjFileName, false);
	models["track"] = TrackModel;

	std::string terrainObjFileName = (currentPath + "\\Models\\Terrain\\terrain.obj");
	Model TerrainModel(terrainObjFileName, false);
	models["terrain"] = TerrainModel;

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	glfwSwapInterval(0);

	float maxDeltaTime = 0.1f;

	while (!glfwWindowShouldClose(window)) 
	{
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		deltaTime = deltaTime > maxDeltaTime ? maxDeltaTime : deltaTime;
		lastFrame = currentFrame;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		processInput(window);

		lightPos.x = 5.0 * cos(glfwGetTime());
		lightPos.z = 5.0 * sin(glfwGetTime());

		RenderScene(shaderManager, models, karts, skybox);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

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

	//Parcurgem toate kart-urile pentru a aplica decelerarea
	for (size_t i = 0; i < karts.size(); ++i) {
		auto& kart = karts[i]; // Referință la kart-ul curent

		if (kart.kartAcceleration > 0.0f) {
			kart.kartAccelerationChanged = true;
			kart.kartAcceleration -= kartDecelerationRate * static_cast<float>(deltaTime);
			if (kart.kartAcceleration < 0.0f)
				kart.kartAcceleration = 0.0f;
		}
		else if (kart.kartAcceleration < 0.0f) {
			kart.kartAccelerationChanged = true;
			kart.kartAcceleration += kartDecelerationRate * static_cast<float>(deltaTime);
			if (kart.kartAcceleration > 0.0f)
				kart.kartAcceleration = 0.0f;
		}
		else {
			kart.kartAccelerationChanged = false;
		}
			

		if (kart.kartAccelerationChanged)
		{
			glm::vec3 kartDirection = glm::vec3(
				sin(glm::radians(kart.rotationAngle)),
				0.0f,
				cos(glm::radians(kart.rotationAngle))
			);

			kart.position += kartDirection * kart.kartAcceleration * static_cast<float>(deltaTime);
		}		
	}

	// Aplicați mișcările doar pentru kart-ul curent
	glm::vec3 kartDirection(
		sin(glm::radians(karts[currentKartIndex].rotationAngle)),
		0.0f,
		cos(glm::radians(karts[currentKartIndex].rotationAngle))
	);

	// Mișcare înainte
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		karts[currentKartIndex].kartAcceleration += kartAccelerationRate * static_cast<float>(deltaTime);
		if (karts[currentKartIndex].kartAcceleration > kartMaxSpeed)
			karts[currentKartIndex].kartAcceleration = kartMaxSpeed;
	}

	// Mișcare înapoi
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		karts[currentKartIndex].kartAcceleration -= kartAccelerationRate * static_cast<float>(deltaTime);
		if (karts[currentKartIndex].kartAcceleration < -kartMaxSpeed / 2.0f)
			karts[currentKartIndex].kartAcceleration = -kartMaxSpeed / 2.0f;
	}

	// Actualizează poziția kart-ului curent
	karts[currentKartIndex].position += kartDirection * karts[currentKartIndex].kartAcceleration * static_cast<float>(deltaTime);

	// Rotație la stânga
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		float rotationFactor = (karts[currentKartIndex].kartAcceleration >= 0.0f) ? 1.0f : -1.0f; // Schimbă sensul rotației
		karts[currentKartIndex].rotationAngle += kartRotationSpeed * rotationFactor * static_cast<float>(deltaTime);
	}

	// Rotație la dreapta
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		float rotationFactor = (karts[currentKartIndex].kartAcceleration >= 0.0f) ? 1.0f : -1.0f; // Schimbă sensul rotației
		karts[currentKartIndex].rotationAngle -= kartRotationSpeed * rotationFactor * static_cast<float>(deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		pCamera->SetCameraMode(CameraMode::FREE);  // Mișcare libera a camerei

	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
	{
		pCamera->SetCameraMode(CameraMode::FIRST_PERSON);  // Modul first person
	}

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		pCamera->SetCameraMode(CameraMode::THIRD_PERSON);  // Modul third person
	}

	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
	{
		lastKartIndex = currentKartIndex;
		if (currentKartIndex > 0)
			currentKartIndex--;
		else
			currentKartIndex = karts.size() - 1;
		glfwWaitEventsTimeout(1);
	}

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
	{
		lastKartIndex = currentKartIndex;
		if (currentKartIndex < karts.size() - 1)
			currentKartIndex++;
		else
			currentKartIndex = 0;
		glfwWaitEventsTimeout(1);
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);
	}
}

void RenderScene(ShaderManager& shaderManager, std::unordered_map<std::string, Model>& models, std::vector<Kart>& karts, std::shared_ptr<SkyBox> skyBoxInstance)
{
	RenderSkybox(shaderManager.GetShader("skyboxShader"),skyBoxInstance);
	RenderKarts(shaderManager.GetShader("lightingWithTextureShader"),karts);
	RenderTrack(shaderManager.GetShader("lightingWithTextureShader"), models.at("track"));
	RenderTerrain(shaderManager.GetShader("lightingWithTextureShader"), models.at("terrain"));
}

void RenderSkybox(Shader& shader, std::shared_ptr<SkyBox> skyBoxInstance)
{
	shader.use();
	shader.setMat4("projection", pCamera->GetProjectionMatrix());
	shader.setMat4("view", glm::mat4(glm::mat3(pCamera->GetViewMatrix())));
	skyBoxInstance->Render();
}

void RenderKarts(Shader& shader, std::vector<Kart>& karts)
{
	shader.use();
	shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
	shader.SetVec3("lightPos", lightPos);
	shader.SetVec3("viewPos", pCamera->GetPosition());
	shader.setInt("texture_diffuse1", 0);

	shader.setMat4("projection", pCamera->GetProjectionMatrix());
	shader.setMat4("view", pCamera->GetViewMatrix());

	// Setează kart-ul curent
	karts[currentKartIndex].isCurrentPlayer = true;

	// Resetează kart-urile anterioare
	if (currentKartIndex != lastKartIndex && lastKartIndex >= 0)
		karts[lastKartIndex].isCurrentPlayer = false;

	// Itierează prin toate karturile și pilotele
	for (Kart& kart : karts) {
		glm::mat4 go_kartModel = glm::mat4(1.0f);
		go_kartModel = glm::translate(go_kartModel, kart.position);       // Aplică poziția kart-ului
		go_kartModel = glm::rotate(go_kartModel, glm::radians(kart.rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Aplică rotația
		go_kartModel = glm::scale(go_kartModel, glm::vec3(0.06f));  // Aplică scala kart-ului
		if (kart.isCurrentPlayer) {
			glm::vec3 kartForwardVector = glm::normalize(glm::vec3(go_kartModel[2]));  // Obține vectorul de direcție al kart-ului
			pCamera->UpdateKartPosition(kart.position, kartForwardVector); // Actualizează poziția camerei
		}

		shader.use();
		shader.setMat4("model", go_kartModel);
		kart.kartModel.Draw(shader);

		if (kart.isCurrentPlayer)
		{
			kart.pilotModel->UpdatePosition(kart.position, kart.rotationAngle);
			glm::mat4 pilotModel = glm::mat4(1.0f);
			pilotModel = glm::translate(pilotModel, glm::vec3(0.0f, 1.2f, 0.0f)); // Poziționează pilotul în kart
			pilotModel = go_kartModel * pilotModel; // Aplică rotația kart-ului la pilot
			pilotModel = glm::scale(pilotModel, glm::vec3(0.02f)); // Ajustează scala pilotului
			shader.setMat4("model", pilotModel);
			kart.pilotModel->Draw(shader);
		}
	}
}

void RenderTerrain(Shader& shader, Model& model)
{
	shader.use();
	shader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
	shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
	shader.SetVec3("lightPos", lightPos);
	shader.SetVec3("viewPos", pCamera->GetPosition());
	shader.setInt("texture_diffuse1", 0);

	shader.setMat4("projection", pCamera->GetProjectionMatrix());
	shader.setMat4("view", pCamera->GetViewMatrix());

	glm::mat4 terrainModel = glm::mat4(1.0f);
	terrainModel = glm::scale(terrainModel, glm::vec3(0.1f)); // Ajustează scala terenului
	terrainModel = glm::translate(terrainModel, glm::vec3(0.0f, -3800.0f, 0.0f)); // Poziționează terenul
	shader.setMat4("model", terrainModel);
	model.Draw(shader);
}

void LoadMultipleKarts(std::vector<Kart>& karts, std::unordered_map<std::string, Model>& models, std::shared_ptr<Pilot> pilot)
{
	Kart kart1;
	kart1.position = kartPos + glm::vec3(0.0f, 0.0f, 0.0f);
	kart1.rotationAngle = kartRotationAngle + 0.0f;
	kart1.kartModel = models.at("go_kart");
	kart1.pilotModel = pilot;
	kart1.kartAcceleration = kartAcceleration;
	karts.push_back(kart1);

	Kart kart2;
	kart2.position = kartPos + glm::vec3(34.0f, 0.0f, 0.0f);
	kart2.rotationAngle = kartRotationAngle + 0.0f;
	kart2.kartModel = models.at("go_kart");
	kart2.pilotModel = pilot;
	kart2.kartAcceleration = kartAcceleration;
	karts.push_back(kart2);

	Kart kart3;
	kart3.position = kartPos + glm::vec3(0.0f, 0.0f, 50.0f);
	kart3.rotationAngle = kartRotationAngle + 180.0f;
	kart3.kartModel = models.at("go_kart");
	kart3.pilotModel = pilot;
	kart3.kartAcceleration = kartAcceleration;
	karts.push_back(kart3);

	Kart kart4;
	kart4.position = kartPos + glm::vec3(34.0f, 0.0f, 50.0f);
	kart4.rotationAngle = kartRotationAngle + 180.0f;
	kart4.kartModel = models.at("go_kart");
	kart4.pilotModel = pilot;
	kart4.kartAcceleration = kartAcceleration;
	karts.push_back(kart4);

	Kart kart5;
	kart5.position = kartPos + glm::vec3(116.0f, 0.0f, 0.0f);
	kart5.rotationAngle = kartRotationAngle + 0.0f;
	kart5.kartModel = models.at("go_kart");
	kart5.pilotModel = pilot;
	kart5.kartAcceleration = kartAcceleration;
	karts.push_back(kart5);

	Kart kart6;
	kart6.position = kartPos + glm::vec3(150.0f, 0.0f, 0.0f);
	kart6.rotationAngle = kartRotationAngle + 0.0f;
	kart6.kartModel = models.at("go_kart");
	kart6.pilotModel = pilot;
	kart6.kartAcceleration = kartAcceleration;
	karts.push_back(kart6);

	Kart kart7;
	kart7.position = kartPos + glm::vec3(116.0f, 0.0f, 50.0f);
	kart7.rotationAngle = kartRotationAngle + 180.0f;
	kart7.kartModel = models.at("go_kart");
	kart7.pilotModel = pilot;
	kart7.kartAcceleration = kartAcceleration;
	karts.push_back(kart7);

	Kart kart8;
	kart8.position = kartPos + glm::vec3(150.0f, 0.0f, 50.0f);
	kart8.rotationAngle = kartRotationAngle + 180.0f;
	kart8.kartModel = models.at("go_kart");
	kart8.pilotModel = pilot;
	kart8.kartAcceleration = kartAcceleration;
	karts.push_back(kart8);

	Kart kart9;
	kart9.position = kartPos + glm::vec3(215.0f, 0.0f, 0.0f);
	kart9.rotationAngle = kartRotationAngle + 0.0f;
	kart9.kartModel = models.at("go_kart");
	kart9.pilotModel = pilot;
	kart9.kartAcceleration = kartAcceleration;
	karts.push_back(kart9);

	Kart kart10;
	kart10.position = kartPos + glm::vec3(249.0f, 0.0f, 0.0f);
	kart10.rotationAngle = kartRotationAngle + 0.0f;
	kart10.kartModel = models.at("go_kart");
	kart10.pilotModel = pilot;
	kart10.kartAcceleration = kartAcceleration;
	karts.push_back(kart10);

	Kart kart11;
	kart11.position = kartPos + glm::vec3(215.0f, 0.0f, 50.0f);
	kart11.rotationAngle = kartRotationAngle + 180.0f;
	kart11.kartModel = models.at("go_kart");
	kart11.pilotModel = pilot;
	kart11.kartAcceleration = kartAcceleration;
	karts.push_back(kart11);

	Kart kart12;
	kart12.position = kartPos + glm::vec3(249.0f, 0.0f, 50.0f);
	kart12.rotationAngle = kartRotationAngle + 180.0f;
	kart12.kartModel = models.at("go_kart");
	kart12.pilotModel = pilot;
	kart12.kartAcceleration = kartAcceleration;
	karts.push_back(kart12);
}

bool checkCollision(const Kart& kart1, const Kart& kart2)
{
	glm::vec3 kart1Center = kart1.position + kart1.kartModel.boundingSphereCenter;
	glm::vec3 kart2Center = kart2.position + kart2.kartModel.boundingSphereCenter;

	float kart1Radius = kart1.kartModel.boundingSphereRadius;
	float kart2Radius = kart2.kartModel.boundingSphereRadius;

	float distance = glm::distance(kart1Center, kart2Center);

	return distance <= (kart1Radius + kart2Radius);
}

void handleCollision(Kart& kart1, Kart& kart2) {
	// Presupunem că fiecare kart are o masă constantă (de exemplu 1.0f)
	float mass1 = 1.0f;
	float mass2 = 1.0f;

	// Calculăm viteza curentă pe baza accelerației și direcției
	glm::vec3 velocity1 = kart1.kartDirection * kart1.kartAcceleration;
	glm::vec3 velocity2 = kart2.kartDirection * kart2.kartAcceleration;

	// Momentul fiecărui kart
	glm::vec3 momentum1 = mass1 * velocity1;
	glm::vec3 momentum2 = mass2 * velocity2;

	// Conservarea momentului: calculăm viteza finală combinată
	glm::vec3 totalMomentum = momentum1 + momentum2;

	// Coeficient de coliziune elastică (poți ajusta acest coeficient pentru diferite efecte)
	float restitution = 0.8f; // Coeficient de restaurare (elasticitate)

	// Aplicați efectul coliziunii asupra fiecărei viteze
	glm::vec3 newVelocity1 = totalMomentum / mass1 * restitution;
	glm::vec3 newVelocity2 = totalMomentum / mass2 * restitution;

	// Actualizează direcțiile karturilor
	kart1.kartDirection = glm::normalize(newVelocity1);
	kart2.kartDirection = glm::normalize(newVelocity2);

	// Actualizează accelerațiile (viteza)
	kart1.kartAcceleration = glm::length(newVelocity1);
	kart2.kartAcceleration = glm::length(newVelocity2);
}

void RenderTrack(Shader& shader, Model& model)
{
	shader.use();
	shader.SetVec3("objectColor", 0.5f, 1.0f, 0.31f);
	shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
	shader.SetVec3("lightPos", lightPos);
	shader.SetVec3("viewPos", pCamera->GetPosition());
	shader.setInt("texture_diffuse1", 0);

	shader.setMat4("projection", pCamera->GetProjectionMatrix());
	shader.setMat4("view", pCamera->GetViewMatrix());

	glm::mat4 trackModel = glm::mat4(1.0f);
	trackModel = glm::scale(trackModel, glm::vec3(200.0f)); // Ajustează scala pistei
	trackModel = glm::translate(trackModel, glm::vec3(0.0f, -0.008f, 0.0f)); // Poziționează pista
	shader.setMat4("model", trackModel);
	model.Draw(shader);
}

