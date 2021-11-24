#pragma once

#include "Texture.h"

#include <array>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

class GUI
{
public:
	void run();

	static GUI* get();

public:
	struct GUIVars
	{
		float osc1Volume = 0.5, osc2Volume = 0.5;
		float lfo1Rate = 1, lfo2Rate = 1;
		float lfo1Amount = 0, lfo2Amount = 0;
		float aeAttack = 0.5, aeDecay = 0.5, aeSustain = 0.5, aeRelease = 0.5;
		float osc1Shift = 0, osc2Shift = 0;
		int osc1Shape = 0, osc2Shape = 0;
		int lfo1Shape = 0, lfo2Shape = 0;

		float* lfo1Target = nullptr, * lfo2Target = nullptr;
		float lfo1Initial, lfo2Initial;
	} vars;

private:
	GUI();
	GUI(const GUI&) {}
	~GUI();

	void process();
	void helpBox(const char* msg);
	void lfoTargetCombo(const char* name, const char** currentItem, float*& target, float& targetInitial);
	void waveShapeSelectGUI(int* target);

private:
	static GUI* instance;
	std::array<Texture, 4> tinyWaveShapeTextures;
};