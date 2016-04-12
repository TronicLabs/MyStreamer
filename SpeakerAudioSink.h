#pragma once

#include <atlbase.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <Windows.h>

class SpeakerAudioSink {
public:
	SpeakerAudioSink(PWAVEFORMATEX format);
	~SpeakerAudioSink();
	void write(PBYTE data, UINT32 num_frames);
private:
	CComPtr<IAudioClient> client_;
	CComPtr<IAudioRenderClient> render_client_;
	HANDLE event_handle_;
};
