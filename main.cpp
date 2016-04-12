#include <objbase.h>

#include "LoopbackAudioSource.h"
#include "SpeakerAudioSink.h"

int main() {
	CoInitializeEx(nullptr, 0);

	LoopbackAudioSource source;
	SpeakerAudioSink sink(source.format());
	while (true) {
		LoopbackAudioSource::Data data = source.read();
		sink.write(data, data.num_frames());
	}
}
