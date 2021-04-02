#pragma once

#include "RtAudio.h"
#include "RtMidi.h"
#include <functional>
#include <iostream>

namespace audio_midi_helper
{
    enum
    {
        MidiMessageNoteOn,
        MidiMessageNoteOff
    };

    struct MidiMessage
    {
        int messageType;
        int note;
    };

    class AudioMidiHelper
    {
    public:
        using audio_callback_t = std::function<void(float *, float *, int)>;
        using midi_callback_t = std::function<void(MidiMessage)>;

        struct Config
        {
            unsigned int sample_rate;
            unsigned int buffer_size;
            bool stay_open;
            audio_callback_t *audio_callback;
            midi_callback_t *midi_callback;
        };

        AudioMidiHelper(Config config)
            : config(config)
        {
            if (config.audio_callback != nullptr)
            {
                start_audio();
            }

            if (config.midi_callback != nullptr)
            {
                start_midi();
            }

            if (config.stay_open)
            {
                getchar();
            }
        }

        ~AudioMidiHelper()
        {
            if (config.audio_callback != nullptr)
            {
                stop_audio();
            }
        }

    private:
        Config config;

        RtAudio dac;
        RtMidiIn midi_in;
        RtMidiOut midi_out;

        void start_audio()
        {
            if (dac.getDeviceCount() < 1)
            {
                std::cout << "\nNo audio devices found!\n";
                exit(0);
            }
            RtAudio::StreamParameters parameters;
            parameters.deviceId = dac.getDefaultOutputDevice();
            parameters.nChannels = 2;
            parameters.firstChannel = 0;

            try
            {
                dac.openStream(&parameters, NULL, RTAUDIO_FLOAT32, config.sample_rate, &config.buffer_size, rtaudio_callback, static_cast<void *>(&config));
                dac.startStream();
            }
            catch (RtAudioError &e)
            {
                e.printMessage();
                exit(0);
            }
        }

        void stop_audio()
        {
            try
            {
                dac.stopStream();
            }
            catch (RtAudioError &e)
            {
                e.printMessage();
            }

            if (dac.isStreamOpen())
            {
                dac.closeStream();
            }
        }

        void start_midi()
        {
            // TODO: support midi out

            unsigned int nPorts = midi_in.getPortCount();
            if (nPorts == 0)
            {
                std::cout << "No midi ports available!\n";
                exit(1);
            }

            midi_in.openPort(0);
            midi_in.ignoreTypes(true, true, true);
            midi_in.setCallback(rtmidi_callback, &config);
        }

        static int rtaudio_callback(void *outputBuffer,
                                    void *inputBuffer,
                                    unsigned int nBufferFrames,
                                    double streamTime,
                                    RtAudioStreamStatus status,
                                    void *userData)
        {
            float *input = static_cast<float *>(inputBuffer);
            float *output = static_cast<float *>(outputBuffer);

            auto config = static_cast<Config *>(userData);
            auto callback = *(config->audio_callback);
            callback(input, output, nBufferFrames);

            return 0;
        }

        static void rtmidi_callback(double deltatime, std::vector<unsigned char> *message, void *userData)
        {
            // TODO: make logging configurable

            auto config = static_cast<Config *>(userData);
            auto callback = *(config->midi_callback);

            unsigned int nBytes = message->size();

            int status = message->at(0);

            // note on
            if ((status & 0xf0) == 0x90)
            {
                int channel = (status & 0xf);
                int note = message->at(1);

                callback(MidiMessage{MidiMessageNoteOn, note});

                std::cout
                    << "ch:" << channel << " - note on" << std::endl;
            }

            // note off
            else if ((status & 0xf0) == 0x80)
            {
                int channel = (status & 0xf);
                int note = message->at(1);

                callback(MidiMessage{MidiMessageNoteOff, note});

                std::cout << "ch:" << channel << " - note off" << std::endl;
            }

            for (unsigned int i = 0; i < nBytes; i++)
            {
                auto msg = message->at(i);
                std::cout << "Byte " << i << " = " << (int)msg << ", ";
            }
            if (nBytes > 0)
                std::cout << "stamp = " << deltatime << std::endl;
        };
    };
}

