#pragma once

#include <memory>

#include <ggl/asset.h>

#include <AL/alc.h>
#include <AL/al.h>

#include <vorbis/vorbisfile.h>

#include <ggl/audio_player.h>

namespace ggl { namespace oal {

class audio_player : public ggl::audio_player
{
public:
	audio_player();
	~audio_player();

	void open(const std::string& path) override;
	void close() override;

	void start() override;
	void stop() override;

	void update() override;

	void set_gain(float g);

private:
	static size_t read(void *ptr, size_t size, size_t nmemb, void *datasource);
	size_t read(void *ptr, size_t size, size_t nmemb);

	class buffer;
	buffer *get_buffer(ALuint id);

	struct buffer {
                buffer();
                ~buffer();

                long load(OggVorbis_File *stream);
                void queue(ALuint source, ALenum format, int rate);

		static const int BUFFER_SIZE = 2*8192;
                char data_[BUFFER_SIZE];

                long size_;
                ALuint id_;
        };

	static const int NUM_BUFFERS = 4;
        buffer buffers_[NUM_BUFFERS];

	std::unique_ptr<ggl::asset> ogg_asset_;
        OggVorbis_File ogg_stream_;

        ALuint source_;

        ALenum format_;
        int rate_;
        int num_samples_;

	float gain_;
	bool playing_;
};

} }
