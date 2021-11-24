#include <iostream>
#include <vector>

#include <soundio/soundio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Texture.h"
#include "App.h"
#include "Synth.h"

static GLFWwindow* window;
static SoundIo* soundio;
static SoundIoDevice* device;
static SoundIoOutStream* outstream;

static int initializeSoundSystem()
{
	std::cout << "Initializing sound system..." << std::endl;

	int err;
	soundio = soundio_create();
	if (!soundio) 
	{
		fprintf(stderr, "out of memory\n");
		return -1;
	}

	if ((err = soundio_connect(soundio))) 
	{
		fprintf(stderr, "error connecting: %s", soundio_strerror(err));
		return -1;
	}

	soundio_flush_events(soundio);

	int default_out_device_index = soundio_default_output_device_index(soundio);
	if (default_out_device_index < 0)
	{
		fprintf(stderr, "no output device found");
		return -1;
	}

	device = soundio_get_output_device(soundio, default_out_device_index);
	if (!device)
	{
		fprintf(stderr, "out of memory");
		return -1;
	}

	fprintf(stderr, "Output device: %s\n", device->name);

	outstream = soundio_outstream_create(device);
	outstream->format = SoundIoFormatFloat32NE;
	outstream->write_callback = Audio::_write_callback;

	if ((err = soundio_outstream_open(outstream))) 
	{
		fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
		return -1;
	}

	if (outstream->layout_error)
	{
		fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));
		return -1;
	}

	if ((err = soundio_outstream_start(outstream))) 
	{
		fprintf(stderr, "unable to start device: %s", soundio_strerror(err));
		return -1;
	}

	return 0;
}

static int initializeWindowSystem()
{
	std::cout << "Initializing window system..." << std::endl;

	if (!glfwInit())
	{
		std::cout << "GLFW init failed!" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(1280, 720, "Synthesizer", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Window is null!" << std::endl;
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Glad init failed!" << std::endl;
		return -1;
	}

	return 0;
}

static int initializeGUISystem()
{
	std::cout << "Initializing gui system..." << std::endl;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	GUI::get();

	return 0;
}

static void terminateSoundSystem()
{
	std::cout << "Terminating sound system..." << std::endl;

	soundio_outstream_destroy(outstream);
	soundio_device_unref(device);
	soundio_destroy(soundio);
}

static void terminateWindowSystem()
{
	std::cout << "Terminating window system..." << std::endl;

	glfwDestroyWindow(window);
	glfwTerminate();
}

static void terminateGUISystem()
{
	std::cout << "Terminating gui system..." << std::endl;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

int main()
{
	//Initialize core systems
	initializeWindowSystem();
	initializeGUISystem();
	initializeSoundSystem();

	{
		//Create the app
		App app(window);

		while (app.running())
		{
			app.beginFrame();

			glClear(GL_COLOR_BUFFER_BIT);

			//Render ImGUI stuff
			ImGui::Render();
			int display_w, display_h;
			glfwGetFramebufferSize(window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			app.endFrame();

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

	}

	//Terminate core systems
	terminateSoundSystem();
	terminateGUISystem();
	terminateWindowSystem();

	return 0;
}