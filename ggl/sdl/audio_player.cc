#include <ggl/panic.h>
#include <ggl/core.h>
#include <ggl/sdl/audio_player.h>

namespace ggl { namespace oal {

audio_player::audio_player()
: playing_ { false }
, gain_ { 1.f }
{
	alGenSources(1, &source_);
}

audio_player::~audio_player()
{
	close();
	alDeleteSources(1, &source_);
}

void
audio_player::set_gain(float g)
{
        gain_ = g;
        alSourcef(source_, AL_GAIN, gain_);
}

void
audio_player::fade_out(int ttl)
{
	fading_out_ = true;
	fade_out_ttl_ = ttl;
	fade_out_tics_ = 0;
}

size_t
audio_player::read(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return static_cast<audio_player *>(datasource)->read(ptr, size, nmemb);
}

size_t
audio_player::read(void *ptr, size_t size, size_t nmemb)
{
	return ogg_asset_->read(ptr, size*nmemb);
}

void
audio_player::open(const std::string& asset_path)
{
	ogg_asset_ = std::move(ggl::g_core->get_asset(asset_path));

	if (ov_open_callbacks(this, &ogg_stream_, nullptr, 0, { read, nullptr, nullptr, nullptr }) < 0)
		panic("failed to open ogg stream");

	const vorbis_info *info = ov_info(&ogg_stream_, -1);

	switch (info->channels) {
		case 1:
			format_ = AL_FORMAT_MONO16;
			break;

		case 2:
			format_ = AL_FORMAT_STEREO16;
			break;

		default:
			panic("invalid # of channels");
	}

	rate_ = info->rate;

	num_samples_ = ov_pcm_total(&ogg_stream_, -1);

	printf("channels=%d rate=%ld samples=%d\n", info->channels, info->rate, num_samples_);
}

void
audio_player::close()
{
	stop();
	ov_clear(&ogg_stream_);
}

void
audio_player::start()
{
	if (playing_)
		return;

	for (auto& b : buffers_) {
		if (b.load(&ogg_stream_) > 0)
			b.queue(source_, format_, rate_);
	}

	alSourcePlay(source_);

	playing_ = true;
}

void
audio_player::stop()
{
	if (!playing_)
		return;

	alSourceStop(source_);

	ALint num_processed;
	while (alGetSourcei(source_, AL_BUFFERS_PROCESSED, &num_processed), num_processed > 0) {
		ALuint id;
		alSourceUnqueueBuffers(source_, 1, &id);
	}

	ov_raw_seek(&ogg_stream_, 0);

	playing_ = false;
}

void
audio_player::update()
{
	if (!playing_)
		return;

	ALint state;
	alGetSourcei(source_, AL_SOURCE_STATE, &state);

	ALint num_processed;
	while (alGetSourcei(source_, AL_BUFFERS_PROCESSED, &num_processed), num_processed > 0) {
		ALuint id;
		alSourceUnqueueBuffers(source_, 1, &id);

		auto *p = get_buffer(id);
		if (p->load(&ogg_stream_) > 0)
			p->queue(source_, format_, rate_);
	}

	if (state == AL_PLAYING && fading_out_) {
		if (++fade_out_tics_ >= fade_out_ttl_) {
			stop();
			fading_out_ = false;
		} else {
			const float t = static_cast<float>(fade_out_tics_)/fade_out_ttl_;
			alSourcef(source_, AL_GAIN, gain_*(1.f - t));
		}
	}

	if (state != AL_PLAYING) {
		ALint queued;
		alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued);

		if (queued) {
			alSourcePlay(source_);
		} else {
			playing_ = false;
		}
	}
}

audio_player::buffer *
audio_player::get_buffer(ALuint id)
{
	for (int i = 0; i < NUM_BUFFERS; i++) {
		if (buffers_[i].id_ == id)
			return &buffers_[i];
	}

	panic("eh? invalid buffer %d?", id);
	return nullptr;
}

audio_player::buffer::buffer()
{
	alGenBuffers(1, &id_);
}

audio_player::buffer::~buffer()
{
	alDeleteBuffers(1, &id_);
}

long
audio_player::buffer::load(OggVorbis_File *stream)
{
	size_ = 0;

	while (size_ < BUFFER_SIZE) {
		int section;
		long r = ov_read(stream, data_ + size_, BUFFER_SIZE - size_, 0, 2, 1, &section);

		if (r < 0)
			panic("ov_read failed");
		else if (r == 0)
			break;

		size_ += r;
	}

	return size_;
}

void
audio_player::buffer::queue(ALuint source, ALenum format, int rate)
{
	if (size_ > 0) {
		alBufferData(id_, format, data_, size_, rate);
		alSourceQueueBuffers(source, 1, &id_);
	}
}

} }
