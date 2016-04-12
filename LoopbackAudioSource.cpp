#include <cassert>

#include "LoopbackAudioSource.h"

LoopbackAudioSource::LoopbackAudioSource()
{
	HRESULT hr;
	CComPtr<IMMDeviceEnumerator> enumerator;
	hr = enumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
	assert(SUCCEEDED(hr));
	CComPtr<IMMDevice> device;
	hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
	assert(SUCCEEDED(hr));

	hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&event_client_);
	assert(SUCCEEDED(hr));
	hr = event_client_->GetMixFormat(&format_);
	assert(SUCCEEDED(hr));
	hr = event_client_->Initialize(AUDCLNT_SHAREMODE_SHARED,
		                           AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
		                           0, 0, format_, nullptr);
	assert(SUCCEEDED(hr));
	event_handle_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	assert(event_handle_);
	hr = event_client_->SetEventHandle(event_handle_);
	assert(SUCCEEDED(hr));
	hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void **)&loopback_client_);
	assert(SUCCEEDED(hr));
	hr = loopback_client_->Initialize(AUDCLNT_SHAREMODE_SHARED,
		                              AUDCLNT_STREAMFLAGS_LOOPBACK,
		                              0, 0, format_, nullptr);
	assert(SUCCEEDED(hr));
	hr = loopback_client_->GetService(__uuidof(IAudioCaptureClient), (void **)&capture_client_);
	assert(SUCCEEDED(hr));
	hr = event_client_->Start();
	assert(SUCCEEDED(hr));
	hr = loopback_client_->Start();
	assert(SUCCEEDED(hr));
}

LoopbackAudioSource::~LoopbackAudioSource()
{
	BOOL ret = CloseHandle(event_handle_);
	assert(ret);
}

LoopbackAudioSource::Data LoopbackAudioSource::read()
{
	PBYTE data;
	UINT32 num_frames;
	DWORD flags;
	HRESULT hr;
	while (true) {
		hr = capture_client_->GetBuffer(&data, &num_frames, &flags, nullptr, nullptr);
		assert(SUCCEEDED(hr) || hr == AUDCLNT_E_OUT_OF_ORDER);
		if (hr == S_OK) {
			return Data(capture_client_, data, num_frames);
		}
		DWORD ret = WaitForSingleObject(event_handle_, INFINITE);  // TODO(iceboy): Event & event multiplex.
		assert(ret == WAIT_OBJECT_0);
	}
}

LoopbackAudioSource::Data::Data(CComPtr<IAudioCaptureClient> &capture_client, PBYTE data, UINT32 num_frames)
	: capture_client_(capture_client)
	, data_(data)
	, num_frames_(num_frames)
{}
