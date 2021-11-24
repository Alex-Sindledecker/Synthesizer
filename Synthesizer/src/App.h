#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <array>
#include <unordered_map>

#include "GUI.h"
#include "Synth.h"

constexpr int MAX_VOICES = 10;

class App
{
public:
	App(GLFWwindow* window);
	~App();

	bool running() const;
	void beginFrame();
	void endFrame();

private:
	GLuint createVertexArray(float* buffer, size_t bufferSize);
	void genWaveTexture(Audio::WaveGenerator generator, double frequency);

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
	double frameStartTime, dt, totalTime;
	GLFWwindow* window;
	Texture waveformTexture, spectrumTexture;
	GLuint quadVao1, quadVao2, shaderProgram;
	GUI* gui;

	static Audio::Synthesizer synthesizer;
	static const Audio::WaveGenerator generators[4];
	static const std::unordered_map<int, double> frequencies;
};