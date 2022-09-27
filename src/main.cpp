// Standard Library
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <random>
#include <chrono>
#include <filesystem>


// 3rd Party Libraries
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glew.h>
#include <GL/GLU.h>


// Basic info about window
static std::string g_caption = "Triangle Example";
static uint32_t g_width = 1280;
static uint32_t g_height = 720;
// Handle Heartbeat
static bool g_running = true;
// Update Timing
static uint32_t g_pre_time = 0;
static uint32_t g_cur_time = 0;
static float g_delta = 0;
// SDL Specific Items
static SDL_Window* g_window = nullptr;
static SDL_GLContext g_context = nullptr;
static SDL_Event g_event;



void demo_init();
void demo_handleEvent(SDL_Event* e);
void demo_update(float delta);
void demo_render();
void demo_release();


int main(int argc, char** argv) {


	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);


	g_window = SDL_CreateWindow(
		g_caption.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		g_width,
		g_height,
		SDL_WINDOW_OPENGL);


	g_context = SDL_GL_CreateContext(g_window);
	glewInit();

	demo_init();



	g_pre_time = SDL_GetTicks();


	while (g_running) {
		g_cur_time = SDL_GetTicks();
		g_delta = (g_cur_time - g_pre_time) / 1000.0f;
		g_pre_time = g_cur_time;


		while (SDL_PollEvent(&g_event)) {

			if (g_event.type == SDL_QUIT) {
				g_running = false;
			}

			demo_handleEvent(&g_event);
		}


		demo_update(g_delta);
		demo_render();

		SDL_GL_SwapWindow(g_window);
	}

	demo_release();

	SDL_GL_DeleteContext(g_context);
	SDL_DestroyWindow(g_window);
	SDL_Quit();

	return 0;
}


static float yrot = 0.0f;
static float yrot_speed = 64.0f;

// Shader
static uint32_t shader_vertex = 0;
static uint32_t shader_fragment = 0;

// Program
static uint32_t program = 0;

// Vertex Array
static uint32_t vertex_array = 0;

// Uniforms
static uint32_t u_proj = 0;
static uint32_t u_view = 0;
static uint32_t u_model = 0;

// Attributes
static const uint32_t A_VERTICES = 0;
static const uint32_t A_COLORS = 1;


// Buffers

// Vertices
static std::vector<glm::vec3> b_vertice_list;
static uint32_t b_vertices_id = 0;

// Colors
static std::vector<glm::vec4> b_colors_list;
static uint32_t b_colors_id = 0;

void loadFile(std::string path, std::function<void(std::string)> cb) {
	std::ifstream in(path);
	std::string temp;
	while (std::getline(in, temp)) {
		cb(temp);
	}
	in.close();
}

uint32_t createShader(GLenum type, std::string path) {
	uint32_t temp = glCreateShader(type);

	std::stringstream ss;

	loadFile(path, [&](std::string line) {
		ss << line << "\n";
	});

	std::string src = ss.str();
	const char* c_src = src.c_str();


	glShaderSource(temp, 1, &c_src, nullptr);

	glCompileShader(temp);


	int len = 0;

	glGetShaderiv(temp, GL_INFO_LOG_LENGTH, &len);

	if (len > 0) {
		std::string log;
		log.resize(len);
		glGetShaderInfoLog(temp, len, nullptr, log.data());
		std::cout << log << "\n";
		log.clear();
	}

	return temp;
}

uint32_t createProgram(const std::vector<uint32_t> shaders) {
	uint32_t temp = glCreateProgram();

	std::for_each(shaders.begin(), shaders.end(), [&](uint32_t shader) {
		glAttachShader(temp, shader);
	});

	glLinkProgram(temp);

	int len = 0;

	glGetProgramiv(temp, GL_INFO_LOG_LENGTH, &len);

	if (len > 0) {
		std::string log;
		log.resize(len);
		glGetProgramInfoLog(temp, len, nullptr, log.data());
		std::cout << log << "\n";
		log.clear();
	}

	return temp;
}

void deleteProgram(uint32_t program, const std::vector<uint32_t> shaders) {
	std::for_each(shaders.begin(), shaders.end(), [&](uint32_t shader) {
		glDetachShader(program, shader);
	});
	glDeleteProgram(program);
}

/*
	1. Shaders
	2. Program
	3. Vertex Array
	3.5 Setting Uniforms and Attributes
	4. Buffers
*/
void demo_init() {
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);


	// Shaders
	shader_vertex = createShader(GL_VERTEX_SHADER, "data/shaders/main.vs.glsl");
	shader_fragment = createShader(GL_FRAGMENT_SHADER, "data/shaders/main.fs.glsl");

	// Program
	program = createProgram({ shader_vertex, shader_fragment });

	// Vertex Array glCreateVertexArrays
	glGenVertexArrays(1, &vertex_array);


	// Setup Uniforms/Attributes
	glUseProgram(program);

	// Uniforms
	u_proj = glGetUniformLocation(program, "proj");
	u_view = glGetUniformLocation(program, "view");
	u_model = glGetUniformLocation(program, "model");

	// Attributes
	glBindVertexArray(vertex_array);
		glEnableVertexAttribArray(A_VERTICES);
		glEnableVertexAttribArray(A_COLORS);
	glBindVertexArray(0);

	glUseProgram(0);


	// Setup Buffers
	/*
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, -1.0f, 0.0f);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
	*/
	// 
	// Vertices
	glGenBuffers(1, &b_vertices_id);

	b_vertice_list.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
	b_vertice_list.push_back(glm::vec3(1.0f, -1.0f, 0.0f));
	b_vertice_list.push_back(glm::vec3(-1.0f, -1.0f, 0.0f));

	glBindBuffer(GL_ARRAY_BUFFER, b_vertices_id);
	// GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW
	glBufferData(GL_ARRAY_BUFFER, b_vertice_list.size() * sizeof(glm::vec3), b_vertice_list.data(), GL_DYNAMIC_DRAW); // 
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Colors
	glGenBuffers(1, &b_colors_id);

	b_colors_list.push_back(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	b_colors_list.push_back(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	b_colors_list.push_back(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

	glBindBuffer(GL_ARRAY_BUFFER, b_colors_id);
	glBufferData(GL_ARRAY_BUFFER, b_colors_list.size() * sizeof(glm::vec4), b_colors_list.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void demo_handleEvent(SDL_Event* e) {
	// Nothing here for now...
}

void demo_update(float delta) {
	yrot += yrot_speed * delta;

	if (yrot >= 360.0f) {
		yrot -= 360.0f;
	}
}

void demo_render() {

	glm::mat4 proj = glm::perspective(
		glm::radians(45.0f), 
		(float)g_width / (float)g_height,
		1.0f,
		1024.0f);

	glm::mat4 view = glm::mat4(1.0f);

	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// All of these functions are not in opengl 4

	glUseProgram(program);

	glUniformMatrix4fv(u_proj,1,GL_FALSE,&proj[0][0]);

	glUniformMatrix4fv(u_view,1,GL_FALSE,&view[0][0]);

	glm::mat4 model =
		glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f)) *
		glm::rotate(glm::mat4(1.0f), glm::radians(yrot), glm::vec3(0.0f, 1.0f, 0.0f));


	glUniformMatrix4fv(u_model,1,GL_FALSE,&model[0][0]);


	glBindVertexArray(vertex_array);

	glBindBuffer(GL_ARRAY_BUFFER, b_vertices_id);
	glVertexAttribPointer(A_VERTICES, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, b_colors_id);
	glVertexAttribPointer(A_COLORS, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDrawArrays(GL_TRIANGLES, 0, b_vertice_list.size());

	glBindVertexArray(0);

	glUseProgram(0);

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (float)g_width / (float)g_height, 1.0f, 1024.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, 0.0f);

	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -5.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);

	glBegin(GL_TRIANGLES);
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 1.0f, 0.0f);
		glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, -1.0f, 0.0f);
		glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
		glVertex3f(-1.0f, -1.0f, 0.0f);
	glEnd();
	glPopMatrix();
	*/
}

void demo_release() {
	glDeleteBuffers(1, &b_vertices_id);
	glDeleteBuffers(1, &b_colors_id);
	glDeleteVertexArrays(1, &vertex_array);
	deleteProgram(program, { shader_vertex, shader_fragment });
	glDeleteShader(shader_vertex);
	glDeleteShader(shader_fragment);
}
