Header only helper class for quick set up of audio and midi io.

usage:

```c++
using namespace audio_midi_helper;

AudioMidiHelper::audio_callback_t audio_callback = [&](float *input, float *output, int nFrames) {
    // do audio stuff
};

AudioMidiHelper::midi_callback_t midi_callback = [&](MidiMessage message) {
    // do midi stuff
};

AudioMidiHelper::Config config;
config.audio_callback = &audio_callback;
config.midi_callback = &midi_callback;
config.sample_rate = 48000;
config.buffer_size = 512;
config.stay_open = true;

AudioMidiHelper amh(config);
```

dependencies:

- RtAudio
- RtMidi
