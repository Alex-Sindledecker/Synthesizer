#include "Synth.h"

#include <algorithm>
#include <vector>
#include <iostream>

namespace Audio
{

	/*
	void applyEnvelope(Sampler* sampler, EnvelopeProperties& envelope, int sample, bool triggered)
	{
		float time = sample * SECONDS_PER_FRAME;
		if (triggered)
		{
			float attack = envelope.attack;
			float decay = envelope.decay;
			float sustain = envelope.sustain;

			if (time <= attack)
				sampler->envelopeVolume = time / attack;
			else if (time <= decay + attack)
				sampler->envelopeVolume = (time - attack) * -(1 - sustain) / decay + 1;
			else
				sampler->envelopeVolume = sustain;
		}
		else
		{
			float timeOffset = time - sampler->releaseSample * SECONDS_PER_FRAME;
			float sustain = envelope.sustain;
			float release = envelope.release;

			if (timeOffset <= release)
				sampler->envelopeVolume = timeOffset * -sustain / release + sustain;
			else
				sampler->envelopeVolume = 0;
		}
	}
	*/

	
	Voice::Voice()
	{
		volume = 0;
		noteId = -1;
	}

	Voice::~Voice()
	{
	}

	void Voice::startNote(int noteId)
	{
		volume = 1;
		this->noteId = noteId;
	}

	void Voice::stopNote()
	{
		volume = 0;
		noteId = 0;
	}

	void Voice::renderBuffer(AudioBuffer& buffer, int startSample)
	{
		for (int i = 0; i < buffer.size(); i++)
		{
			int sample = startSample + i;
			buffer[i] = sinGenerator(sample, 440);
		}
	}

	bool Voice::isActive()
	{
		return noteId != -1;
	}

	bool Voice::operator<(const Voice& voice)
	{
		return volume < voice.volume;
	}

	Synthesizer::Synthesizer()
	{
	}

	Synthesizer::~Synthesizer()
	{
	}

	bool Synthesizer::renderNextBlock()
	{
		std::lock_guard<std::mutex> g(m);

		bool result = false;

		for (int i = 0; i < voices.size(); i++)
		{
			if (voices[i].isActive())
			{
				voices[i].renderBuffer(finalBuffer, sampleOffsets[i]);
				sampleOffsets[i] += voiceBuffers[i].size();
				result = true;
			}
		}

		return result;
	}

	bool Synthesizer::renderBlockToAreas(SoundIoChannelArea* areas, int numAreas)
	{
		for (int i = 0; i < BUFFER_SIZE; i++)
		{

			float sample = 0;
			for (int j = 0; j < voiceBuffers.size(); j++)
				sample += voiceBuffers[j][i];

			for (int j = 0; j < numAreas; j++)
			{
				double* ptr = (double*)(areas[j].ptr + areas[j].step * i);
				*ptr = sample;
			}
		}
		return false;
	}

	double* Synthesizer::getBlockPtr()
	{
		return finalBuffer.data();
	}

	void Synthesizer::setOscillators(WaveGenerator osc1, WaveGenerator osc2)
	{
	}

	void Synthesizer::setEnvelope(float attack, float decay, float sustain, float release)
	{
	}

	int Synthesizer::createVoice(int noteId)
	{
		voices[0].startNote(noteId);
		return 0;
	}

	Voice* Synthesizer::getVoice(int voiceIndex)
	{
		return &voices[voiceIndex];
	}

	static Synthesizer* _synth = nullptr;

	void setSynthPtr(Synthesizer* synth)
	{
		_synth = synth;
	}

	void _write_callback(SoundIoOutStream* outstream, int frame_count_min, int frame_count_max)
	{
		const SoundIoChannelLayout* layout = &outstream->layout;
		float float_sample_rate = outstream->sample_rate;
		SoundIoChannelArea* areas;
		int frames_left = BUFFER_SIZE;
		int err;

		if (_synth != nullptr)
		{

			while (frames_left > 0) {
				int frame_count = frames_left;

				if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
					fprintf(stderr, "%s\n", soundio_strerror(err));
					exit(1);
				}

				if (!frame_count)
					break;

				if (_synth->renderNextBlock())
				{
					for (int i = 0; i < BUFFER_SIZE; i++)
					{
						for (int channel = 0; channel < layout->channel_count; channel++)
						{
							float* ptr = (float*)(areas[channel].ptr + areas[channel].step * i);
							*ptr = (float)_synth->getBlockPtr()[i];
							int h = 5;
						}
					}
				}
				//_synth->renderBlockToAreas(areas, layout->channel_count);

				if ((err = soundio_outstream_end_write(outstream))) {
					fprintf(stderr, "%s\n", soundio_strerror(err));
					exit(1);
				}

				frames_left -= frame_count;
			}

		}
	}

}