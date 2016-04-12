#pragma once

#include <atlbase.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <Windows.h>

#include <memory>
#include <utility>

class LoopbackAudioSource {
public:
	class Data;
public:
	LoopbackAudioSource();
	~LoopbackAudioSource();
	PWAVEFORMATEX format() { return format_; }
	Data read();
private:
	CComPtr<IAudioClient> event_client_;
	CComPtr<IAudioClient> loopback_client_;
	CComPtr<IAudioCaptureClient> capture_client_;
	HANDLE event_handle_;
	PWAVEFORMATEX format_;
public:
	class Data {
	public:
		Data(CComPtr<IAudioCaptureClient> &capture_client, PBYTE data, UINT32 num_frames);
		~Data() { capture_client_->ReleaseBuffer(num_frames_); }
		operator PBYTE() const { return data_; };
		UINT32 num_frames() const { return num_frames_; };
	private:
		CComPtr<IAudioCaptureClient> &capture_client_;
		PBYTE data_;
		UINT32 num_frames_;
	};
};
