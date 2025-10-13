// Voxel simulation.cpp : Defines the entry point for the application.
//

#include "Voxel simulation.h"

constexpr float WINDOW_WIDTH = 1920.0;
constexpr float WINDOW_HEIGHT = 1080.0;

constexpr int CHUNK_X_DIST = 1000;
constexpr int CHUNK_Z_DIST = 1000;

constexpr GLsizei BYTES_PER_VERTEX = 6;

void drawGUI(Camera& camera, Perlin& perlin, glm::vec3& lightPos, glm::vec3& lightCol);

static float startDistMultiplier = 0.5f;
static float endDistAdd = 300.0f;
static float baseSinkStrength = 8.0f;
static float fadePow = 1.0f;
static float lodScaleDebug = 4.0f;

int main()
{   
	SDL_Window* window;
	bool done = false;
	bool mouse = true;

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialise SDL3: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);


	window = SDL_CreateWindow(
		"Voxel Simulation",
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (window == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GLContext mainContext = SDL_GL_CreateContext(window);
	if (mainContext == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
		return 1;
	}

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))  {
		std::cerr << "Failed to initialize GLAD: " << SDL_GetError() << std::endl;
		return 1;
	}

	if (!SDL_SetWindowRelativeMouseMode(window, mouse)) {
		std::cerr << "FAILED TO SET REALATIVE MOUSE MODE: " << SDL_GetError() << std::endl;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForOpenGL(window, mainContext);
	ImGui_ImplOpenGL3_Init("#version 330");

	SDL_GL_SetSwapInterval(1); 
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Shader stuff
	
	Shader shaderProgram("F:/Projects/Voxel simulation/include/shaders/default.vert", "F:/Projects/Voxel simulation/include/shaders/default.frag");

	// shader end

	Uint32 lastTick = 0;
	Uint32 currentTick = 0;
	GLfloat deltaTime;

	Camera camera;
	Perlin perlin;
	float x, y;

	//Perlin noise;

	Texture texture("F:/Projects/Voxel simulation/include/Textures/ground.jpg", GL_TEXTURE_2D, GL_RGB, GL_UNSIGNED_BYTE);

	Chunk overworld;

	glm::vec3 lightPos = glm::vec3(0, 150, 10);
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	while (!done) {
		lastTick = currentTick;
		currentTick = SDL_GetTicks();
		deltaTime = (currentTick - lastTick) / 100.0f;
		SDL_Event event;

		glm::mat4 view = camera.getViewMatrix();

		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT) {
				done = true;
			}
			if (event.key.key == SDLK_0) {
				mouse = false;
				SDL_SetWindowRelativeMouseMode(window, mouse);
			} 
			if (event.key.key == SDLK_1) {
				mouse = true;
				SDL_SetWindowRelativeMouseMode(window, mouse);
			}
			if (event.type != SDL_EVENT_WINDOW_FOCUS_LOST && mouse == true) {
				SDL_GetRelativeMouseState(&x, &y);
				camera.processMouseMovement(x, y);
			}
		}

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000000.0f);

		glm::mat4 model = glm::mat4(1.0f);
		shaderProgram.setUniform("model", model);

		shaderProgram.setUniform("view", view);
		shaderProgram.setUniform("projection", projection);
		shaderProgram.setUniform("viewPos", camera.Position);
		shaderProgram.setUniform("playerPos", camera.Position);
		shaderProgram.setUniform("lightPos", lightPos);
		shaderProgram.setUniform("lightColor", lightColor);

		shaderProgram.setUniform("startDistMultiplier", startDistMultiplier);
		shaderProgram.setUniform("endDistAdd", endDistAdd);
		shaderProgram.setUniform("baseSinkStrength", baseSinkStrength);
		shaderProgram.setUniform("fadePow", fadePow);


		texture.bindTexture();
		shaderProgram.setUniform("ourTexture", 0);

		shaderProgram.activate();

		overworld.generateChunks(camera.Position, perlin);

		overworld.drawChunks(shaderProgram);


		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		drawGUI(camera, perlin, lightPos, lightColor);

		ImGui::Render();


		glDisable(GL_DEPTH_TEST);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glEnable(GL_DEPTH_TEST);

		
		const bool* keyState = SDL_GetKeyboardState(NULL);

		if (keyState[SDL_SCANCODE_W]) {
			camera.processKeyboard(FORWARD, deltaTime);
		}
		if (keyState[SDL_SCANCODE_S]) {
			camera.processKeyboard(BACKWARD, deltaTime);
		}
		if (keyState[SDL_SCANCODE_A]) {
			camera.processKeyboard(LEFT, deltaTime);
		}
		if (keyState[SDL_SCANCODE_D]) {
			camera.processKeyboard(RIGHT, deltaTime);
		}

		// logic here

		SDL_GL_SwapWindow(window);
	}

	//squareMesh.Delete();
	//overworld.deleteMeshData();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	//shaderProgram.deleteShaderProgram();

	SDL_GL_DestroyContext(mainContext);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}


void drawGUI(Camera& camera, Perlin& perlin, glm::vec3& lightPos, glm::vec3& lightCol) {
	//camera
	ImGui::Begin("Camera");
	ImGui::SliderFloat("Movement speed", &camera.MovementSpeed, 1.0f, 200.0f);
	ImGui::SliderFloat("Sensitivity", &camera.MouseSensitivity, 0.1f, 10.0f);
	ImGui::End();

	float col[3]{ lightCol.x, lightCol.y, lightCol.z };

	//Light
	ImGui::Begin("Shader");
	ImGui::SliderFloat("Light position X", &lightPos.x, -100, 100);
	ImGui::SliderFloat("Light position Y", &lightPos.y, -100, 100);
	ImGui::SliderFloat("Light position Z", &lightPos.z, -100, 100);
	ImGui::ColorEdit3("Light Color", col);

	lightCol.x = col[0];
	lightCol.y = col[1];
	lightCol.z = col[2];

	ImGui::End();

	//Sinking 
	ImGui::Begin("Sinkng");
	ImGui::SliderFloat("StartDist Mult", &startDistMultiplier, 0.0f, 2.0f);
	ImGui::SliderFloat("EndDist Add", &endDistAdd, 0.0f, 1000.0f);
	ImGui::SliderFloat("Base Sink", &baseSinkStrength, 0.0f, 20.0f);
	ImGui::SliderFloat("Fade Power", &fadePow, 0.1f, 5.0f);

	ImGui::End();
}