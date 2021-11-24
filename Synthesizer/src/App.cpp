#include "App.h"

#include <iostream>
#include <vector>
#include <algorithm>

const char* vertexShaderStr =
"#version 330 core\n"\
"layout (location = 0) in vec2 pos;"\
"out vec2 uv;"\
"void main(){"\
"gl_Position = vec4(pos, 0, 1);"\
"uv = vec2((pos.x + 1.f) / 2.f, (pos.y + 1.f) / 2.f);"\
"}";

const char* fragmentShaderStr =
"#version 330 core\n"\
"in vec2 uv;"\
"uniform sampler2D tex;"\
"out vec4 color;"\
"void main(){"\
"color = texture(tex, uv);"
"}";

float quad1[] = {
	1.f, 1.f,
	-1.f, 1.f,
	-1.f, 0.f,

	-1.f, 0.f,
	1.f, 0.f,
	1.f, 1.f,
};

float quad2[] = {
	1.f, 0.f,
	-1.f, 0.f,
	-1.f, -1.f,

	-1.f, -1.f,
	1.f, -1.f,
	1.f, 0.f,
};

Audio::Synthesizer App::synthesizer;
const Audio::WaveGenerator App::generators[4] = { Audio::sinGenerator, Audio::squareGenerator, Audio::sawGenerator, Audio::triangleGenerator };
const std::unordered_map<int, double> App::frequencies = { {GLFW_KEY_A, 440}, {GLFW_KEY_B, 493.88}, {GLFW_KEY_C, 523.25}, {GLFW_KEY_D, 587.33},
	{GLFW_KEY_E, 659.25}, {GLFW_KEY_F, 698.46}, {GLFW_KEY_G, 783.99 } };

App::App(GLFWwindow* window)
{
	this->window = window;
	dt = 0;

	glfwSetWindowUserPointer(window, this);
	glfwSetKeyCallback(window, keyCallback);

	quadVao1 = createVertexArray(quad1, sizeof(quad1));
	quadVao2 = createVertexArray(quad2, sizeof(quad2));

	GLuint vertexShader, fragmentShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderStr, nullptr);
	glCompileShader(vertexShader);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderStr, nullptr);
	glCompileShader(fragmentShader);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glDetachShader(shaderProgram, vertexShader);
	glDetachShader(shaderProgram, fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	Audio::setSynthPtr(&synthesizer);

	gui = GUI::get();
}

App::~App()
{
	glDeleteProgram(shaderProgram);

	glDeleteVertexArrays(1, &quadVao2);
	glDeleteVertexArrays(1, &quadVao1);
}

bool App::running() const
{
	return !glfwWindowShouldClose(window);
}

void App::beginFrame()
{
	frameStartTime = glfwGetTime();

	gui->run();
	genWaveTexture([=](double x, double freq) {
		return gui->vars.osc1Volume * (generators[gui->vars.osc1Shape](x, freq) + 1.0) / 2.0 +
			   gui->vars.osc2Volume * (generators[gui->vars.osc2Shape](x, freq) + 1.0) / 2.0;  }, 
		440);

	if (gui->vars.lfo1Target != nullptr)
	{
		const float amp = gui->vars.lfo1Initial * gui->vars.lfo1Amount / 200.f;
		*gui->vars.lfo1Target = amp * generators[gui->vars.lfo1Shape](totalTime, gui->vars.lfo1Rate) + gui->vars.lfo1Initial - amp;
	}
	if (gui->vars.lfo2Target != nullptr)
	{
		const float amp = gui->vars.lfo2Initial * gui->vars.lfo2Amount / 200.f;
		*gui->vars.lfo2Target = amp * generators[gui->vars.lfo2Shape](totalTime, gui->vars.lfo2Rate) + gui->vars.lfo2Initial - amp;
	}
}

void App::endFrame()
{
	double frameEndTime = glfwGetTime();
	dt = frameEndTime - frameStartTime;
	totalTime += dt;

	glUseProgram(shaderProgram);
	waveformTexture.bind();
	glBindVertexArray(quadVao1);
	//glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

GLuint App::createVertexArray(float* buffer, size_t bufferSize)
{
	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, bufferSize, buffer, GL_STATIC_DRAW);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDeleteBuffers(1, &vbo);

	return vao;
}

void App::genWaveTexture(Audio::WaveGenerator generator, double frequency)
{
	const unsigned int size = 256;
	const unsigned int channels = 4;
	unsigned char pixels[size * size * channels];

	int values[256];
	for (int i = 0; i < 256; i++)
		values[i] = generator(i, 256) * 256;

	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size * channels; x += channels)
		{
			int a = values[x / 4];
			if (y == a)
			{
				pixels[y * size * channels + x + 0] = 0xff;
				pixels[y * size * channels + x + 1] = 0xff;
				pixels[y * size * channels + x + 2] = 0;
				pixels[y * size * channels + x + 3] = 0xff;
			}
			else
			{
				for (int i = 0; i < 4; i++)
					pixels[y * size * channels + x + i] = 0;
			}
		}
	}

	waveformTexture.writePixels(256, 256, pixels);
}

void App::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	App& app = *reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	
	if (frequencies.find(key) != frequencies.end())
	{
		const static int offset = -GLFW_KEY_A;
		if (action == GLFW_PRESS)
		{
			synthesizer.createVoice(key + offset);
		}
		else if (action == GLFW_RELEASE)
		{
		}
	}
}
