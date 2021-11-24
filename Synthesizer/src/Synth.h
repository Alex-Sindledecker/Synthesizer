#pragma once

#include <soundio/soundio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <functional>
#include <array>

#include <mutex>

namespace Audio
{
	constexpr int MAX_VOICES = 8;
	constexpr int SAMPLE_RATE = 44100;
	constexpr int BUFFER_SIZE = 4096;
	constexpr float SECONDS_PER_FRAME = 1.0 / 44100.0;
	constexpr float THRESHOLD = 0.0001;
	constexpr float SEMITONE = 1.05946309;
	constexpr float C0 = 16.35;
	constexpr float PI = 3.14f;
	constexpr float TAU = 2 * PI;

	static float sinGenerator(float x, float freq)
	{
		static float cache = -1;
		static float lastX = -1;
		static float lastFreq = -1;

		if (lastX == x && lastFreq == freq)
			return cache;
		else
		{
			cache = sin(TAU * x * freq);
			lastX = x;
			lastFreq = freq;
			return cache;
		}
	}

	static float squareGenerator(float x, float freq)
	{
		static float cache = -1;
		static float lastX = -1;
		static float lastFreq = -1;

		if (lastX == x && lastFreq == freq)
			return cache;
		else
		{
			cache = 2.0 * (2.0 * floor(freq * x) - floor(2.0 * freq * x)) + 1.0;
			lastX = x;
			lastFreq = freq;
			return cache;
		}
	}

	static float sawGenerator(float x, float freq)
	{
		static float cache = -1;
		static float lastX = -1;
		static float lastFreq = -1;

		if (lastX == x && lastFreq == freq)
			return cache;
		else
		{
			cache = 2.0 * (freq * x - floor(0.5 + freq * x));
			lastX = x;
			lastFreq = freq;
			return cache;
		}
	}

	static float triangleGenerator(float x, float freq)
	{
		return 2 * fabs(sawGenerator(x, freq)) - 1;
	}

	static inline float silentGenerator(float x = 0, float freq = 0)
	{
		return 0;
	}

	using WaveGenerator = std::function<double(double, double)>;
	using AudioBuffer = std::array<double, BUFFER_SIZE>;

	class Voice
	{
	public:
		Voice();
		~Voice();

		void startNote(int noteId);
		void stopNote();
		//Critical
		void renderBuffer(AudioBuffer& buffer, int startSample);
		bool isActive();

		//For sorting
		bool operator<(const Voice& voice);

	private:
		float volume;
		int noteId;
	};

	class Synthesizer
	{
	public:
		Synthesizer();
		~Synthesizer();

		//Critical
		bool renderNextBlock();
		bool renderBlockToAreas(SoundIoChannelArea* areas, int numAreas);
		double* getBlockPtr();
		void setOscillators(WaveGenerator osc1, WaveGenerator osc2);
		void setEnvelope(float attack, float decay, float sustain, float release);
		int createVoice(int noteId);
		Voice* getVoice(int voiceIndex);

	private:
		std::array<Voice, MAX_VOICES> voices;
		std::array<AudioBuffer, MAX_VOICES> voiceBuffers;
		std::array<int, MAX_VOICES> sampleOffsets;
		AudioBuffer finalBuffer;

		WaveGenerator oscillator1, oscillator2;
		float attack, decay, sustain, release;

		std::mutex m;
	};

	void setSynthPtr(Synthesizer* synth);
	void _write_callback(SoundIoOutStream* outstream, int frame_count_min, int frame_count_max);
}