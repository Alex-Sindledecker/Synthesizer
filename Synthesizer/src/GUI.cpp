#include "GUI.h"

#include <iostream>

GUI* GUI::instance;

void GUI::waveShapeSelectGUI(int* target)
{
	ImGui::NewLine();
	ImGui::Columns(4);

	const char* waveShapeText[] = { "Sin", "Square", "Saw", "Triangle" };
	for (int i = 0; i < 4; i++)
	{
		bool selected = (waveShapeText[*target] == waveShapeText[i]);
		if (ImGui::Selectable(waveShapeText[i], selected))
		{
			*target = i;
		}

		float imageX = ImGui::GetCursorPosX() + ImGui::GetColumnWidth() / 2.f - tinyWaveShapeTextures[i].getWidth() / 1.5f;
		ImGui::SetCursorPos(ImVec2(imageX, ImGui::GetCursorPosY()));
		ImGui::Image((void*)(intptr_t)tinyWaveShapeTextures[i].getID(), { (float)tinyWaveShapeTextures[i].getWidth() / 1.5f, (float)tinyWaveShapeTextures[i].getHeight() / 1.5f });
		ImGui::NextColumn();
	}

	ImGui::Columns();
}

void GUI::lfoTargetCombo(const char* name, const char** currentItem, float*& target, float& targetInitial)
{
	static const char* targetStrs[] = { "None", "Oscillator1 Volume", "Oscillator2 Volume", "LFO1 Rate", "LFO2 Rate",
		"LFO1 Amount", "LFO2 Amount", "Amplitude Envelope Attack", "Amplitude Envelope Decay", "Amplitude Envelope Sustain",
		"Amplitude Envelope Release" };

	if (ImGui::BeginCombo(name, *currentItem))
	{
		for (int i = 0; i < IM_ARRAYSIZE(targetStrs); i++)
		{
			bool selected = (*currentItem == targetStrs[i]);
			if (ImGui::Selectable(targetStrs[i], selected))
			{
				*currentItem = targetStrs[i];
				if (i > 0)
				{
					uintptr_t addr = reinterpret_cast<uintptr_t>(&vars.osc1Volume) + (i - 1) * sizeof(float);
					target = reinterpret_cast<float*>(addr);
					targetInitial = *target;
					break;
				}
				else
				{
					target = nullptr;
					targetInitial = 0;
				}
			}
		}
		ImGui::EndCombo();
	}
}

void GUI::helpBox(const char* msg)
{
	ImGui::SameLine(ImGui::GetWindowWidth() - 25);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 0.7));
	ImGui::Text("(?)");
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(msg);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

GUI::GUI()
{
	tinyWaveShapeTextures[0].loadFromImage("res/sinWave.png");
	tinyWaveShapeTextures[1].loadFromImage("res/squarewave.png");
	tinyWaveShapeTextures[2].loadFromImage("res/sawwave.png");
	tinyWaveShapeTextures[3].loadFromImage("res/triangleWave.png");
}

GUI::~GUI()
{
	delete this;
}

void GUI::process()
{
	ImGui::Begin("Synth Controls");

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

	ImGui::BeginTabBar("Synthesizer Tabs");

	if (ImGui::BeginTabItem("Oscillators"))
	{
		//Osc1
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Oscillator 1");
			helpBox("An oscillator is what generates sounds using different wave forms. Each oscillator gets combined to make a unique sound!\
\n\n\nVolume - This controls the volume of the oscillator.\n\n\Pitch Shift - This changes the frequency of the oscillator by a certain amount of \
semitones. A semitone is essentially the distance between two musical notes.\n\nWave Shape - This is what defines the sound of the oscillator.\
Each shape has its own unique sound. Try it!");
			ImGui::BeginChild("Oscillator 1", ImVec2(0, 165), true);
			ImGui::SliderFloat("Volume", &vars.osc1Volume, 0, 1);
			ImGui::SliderFloat("Pitch Shift", &vars.osc1Shift, -12, 12, "%.0f semitones");

			waveShapeSelectGUI(&vars.osc1Shape);

			ImGui::EndChild();
		}

		//Osc2
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Oscillator 2");
			ImGui::BeginChild("Oscillator 2", ImVec2(0, 165), true);
			ImGui::SliderFloat("Volume", &vars.osc2Volume, 0, 1);
			ImGui::SliderFloat("Pitch Shift", &vars.osc2Shift, -12, 12, "%.0f semitones");

			waveShapeSelectGUI(&vars.osc2Shape);

			ImGui::EndChild();
		}

		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("LFOs"))
	{
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Low Frequency Oscillator 1");
			helpBox("A low frequency oscillator (LFO) is exactly like a normal oscillator, but it operatres at a very low frequency that humans can't hear. \
This low frequency oscilation is used to modulate, or control, a parameter somewhere else in the synth. So if you wanted the volume to go up and down slowly, \
you could map the LFO to the volume of the oscillators!\n\n\nRate - This is the frequency the lfo will operate at (how fast it goes up and down).\n\nAmount - \
This is how much the lfo changes the parameter it is mapped to. If it was controlling the volume, and the amount was at 50 percent, then the volume would go up\
50 percent of the remaining volume and back down.\n\nTarget - This is what the LFO will be mapped to and will control.\n\nWave shape - This is exactly the same\
as a normal oscillator wave shape.");
			ImGui::BeginChild("LFO1", ImVec2(0, 165), true);

			ImGui::SliderFloat("Rate", &vars.lfo1Rate, 0.1, 10, "%.3f hz");
			ImGui::SliderFloat("Amount", &vars.lfo1Amount, 0, 100, "%.3f percent");

			static const char* currentItem = "None";
			lfoTargetCombo("LFO1 Target", &currentItem, vars.lfo1Target, vars.lfo1Initial);

			waveShapeSelectGUI(&vars.lfo1Shape);

			ImGui::EndChild();
		}
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Low Frequency Oscillator 2");
			ImGui::BeginChild("LFO2", ImVec2(0, 165), true);

			ImGui::SliderFloat("Rate", &vars.lfo2Rate, 0.1, 10, "%.3f hz");
			ImGui::SliderFloat("Amount", &vars.lfo2Amount, 0, 100, "%.3f percent");

			static const char* currentItem = "None";
			lfoTargetCombo("LFO2 Target", &currentItem, vars.lfo2Target, vars.lfo2Initial);

			waveShapeSelectGUI(&vars.lfo2Shape);

			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
	if (ImGui::BeginTabItem("Envelopes"))
	{
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Amplitude Envelope");
		helpBox("The amplitude envelope changes the volume of the output over time. \n\n\nAttack - The attack is how long it takes the output volume to from 0 to 1.\
\n\nDecay - The decay is how long it takes the output volume to decay from 1 to the sustain volume.\n\nSustain - Sustain is the volume the output will be maintained at\
until a key is released.\n\nRelease - The release is the amount of time it will take to go from the sustain volume back down to 0.\n\n\nNOTE: Most envelopes have \
logarithmic decay time, but this one has linear. This has a noticable effect at higher frequencies.");
		ImGui::BeginChild("Amplitude Envelope", ImVec2(0, 341), true);
		ImGui::SliderFloat("Attack", &vars.aeAttack, 0.001, 2, "%0.3f sec");
		ImGui::SliderFloat("Decay", &vars.aeDecay, 0.001, 1, "%0.3f sec");
		ImGui::SliderFloat("Sustain", &vars.aeSustain, 0, 1);
		ImGui::SliderFloat("Release", &vars.aeRelease, 0.001, 2, "%0.3f sec");

		auto EnvelopeFunc = [](void* data, int i) -> float {
			GUIVars& guiVars = *static_cast<GUIVars*>(data);

			float totalTime = 6.f;
			float iTime = (float)i / 70.f * totalTime;

			if (iTime < guiVars.aeAttack)
			{
				return (1.f / guiVars.aeAttack) * iTime;
			}
			else if (iTime < guiVars.aeAttack + guiVars.aeDecay)
			{
				return -(1 - guiVars.aeSustain) / (guiVars.aeDecay) * (iTime - guiVars.aeAttack) + 1;
			}
			else if (iTime >= totalTime - guiVars.aeRelease)
			{
				return -guiVars.aeSustain / (guiVars.aeRelease) * (iTime - (totalTime - guiVars.aeRelease)) + guiVars.aeSustain;
			}

			return guiVars.aeSustain;
		};

		float (*func)(void*, int) = EnvelopeFunc;
		int display_count = 70;
		ImGui::NewLine();
		ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1, 1, 0, 1));
		ImGui::PlotLines("Volume", func, &vars, display_count, 0, NULL, 0.f, 1.0f, ImVec2(0, 150));
		ImGui::PopStyleColor();
		ImGui::Text("Time");

		ImGui::EndChild();

		ImGui::EndTabItem();
	}

	ImGui::EndTabBar();

	ImGui::PopStyleVar();

	ImGui::End();
}

void GUI::run()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	process();
}

GUI* GUI::get()
{
	if (!instance)
		instance = new GUI();
	return instance;
}