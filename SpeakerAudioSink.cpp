#include <cassert>

#include "SpeakerAudioSink.h"

SpeakerAudioSink::SpeakerAudioSink(PWAVEFORMATEX format)
{
	HRESULT hr;
	CComPtr<IMMDeviceEnumerator> enumerator;
	hr = enumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
	assert(SUCCEEDED(hr));
	CComPtr<IMMDevice> device;
	hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
	assert(SUCCEEDED(hr));

	hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&client_);
	assert(SUCCEEDED(hr));
	hr = client_->Initialize(AUDCLNT_SHAREMODE_SHARED,
		                     AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
		                     0, 0, format, nullptr);
	assert(SUCCEEDED(hr));
	event_handle_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	assert(event_handle_);
	hr = client_->SetEventHandle(event_handle_);
	assert(SUCCEEDED(hr));
	hr = client_->GetService(__uuidof(IAudioRenderClient), (void **)&render_client_);
	assert(SUCCEEDED(hr));
	client_->Start();
	assert(SUCCEEDED(hr));
}

SpeakerAudioSink::~SpeakerAudioSink()
{
	BOOL ret = CloseHandle(event_handle_);
	assert(ret);
}

void SpeakerAudioSink::write(PBYTE data, UINT32 num_frames) {
	PBYTE buffer;
	HRESULT hr;
	while (true) {
		hr = render_client_->GetBuffer(num_frames, &buffer);
		assert(SUCCEEDED(hr));
		if (hr == S_OK) {
			memcpy(buffer, data, num_frames * 8);  // TODO(iceboy): 8
			hr = render_client_->ReleaseBuffer(num_frames, 0);
			assert(SUCCEEDED(hr));
			return;
		}
		DWORD ret = WaitForSingleObject(event_handle_, INFINITE);  // TODO(iceboy): Event & event multiplex.
		assert(ret == WAIT_OBJECT_0);
	}
}
