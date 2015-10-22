#include<stdio.h>
#include <alsa/asoundlib.h>
#include "audio.h"
#include "formats.h"

#define DEFAULT_FORMAT      SND_PCM_FORMAT_U8
#define DEFAULT_SPEED       8000

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif
static size_t pbrec_count = LLONG_MAX;
static size_t bits_per_sample, bits_per_frame;
static size_t chunk_bytes;



ssize_t wavefile(int fd, u_char *_buffer, size_t size, struct audio_params *hwparams)
{
	WaveHeader *h = (WaveHeader *)_buffer;
	u_char *buffer = NULL;
	size_t blimit = 0;
	WaveFmtBody *f;
	WaveChunkHeader *c;
	u_int type, len;
	unsigned short format, channels;
	int big_endian, native_format;

	printf("%s size:%ld\n", __func__, size);
	if (size < sizeof(WaveHeader))
		return -1;
	if (h->magic == WAV_RIFF)
		big_endian = 0;
	else if (h->magic == WAV_RIFX)
		big_endian = 1;
	else
		return -1;
	if (h->type != WAV_WAVE)
		return -1;

	if (size > sizeof(WaveHeader)) {
		check_wavefile_space(buffer, size - sizeof(WaveHeader), blimit);
		memcpy(buffer, _buffer + sizeof(WaveHeader), size - sizeof(WaveHeader));
	}
	size -= sizeof(WaveHeader);
	while (1) {
		check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
		test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
		c = (WaveChunkHeader*)buffer;
		type = c->type;
		len = TO_CPU_INT(c->length, big_endian);
		len += len % 2;
		if (size > sizeof(WaveChunkHeader))
			memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
		size -= sizeof(WaveChunkHeader);
		if (type == WAV_FMT)
			break;
		check_wavefile_space(buffer, len, blimit);
		test_wavefile_read(fd, buffer, &size, len, __LINE__);
		if (size > len)
			memmove(buffer, buffer + len, size - len);
		size -= len;
	}

	if (len < sizeof(WaveFmtBody)) {
		error(_("unknown length of 'fmt ' chunk (read %u, should be %u at least)"),
		      len, (u_int)sizeof(WaveFmtBody));
		prg_exit(EXIT_FAILURE);
	}
	check_wavefile_space(buffer, len, blimit);
	test_wavefile_read(fd, buffer, &size, len, __LINE__);
	f = (WaveFmtBody*) buffer;
	format = TO_CPU_SHORT(f->format, big_endian);
	if (format == WAV_FMT_EXTENSIBLE) {
		WaveFmtExtensibleBody *fe = (WaveFmtExtensibleBody*)buffer;
		if (len < sizeof(WaveFmtExtensibleBody)) {
			error(_("unknown length of extensible 'fmt ' chunk (read %u, should be %u at least)"),
					len, (u_int)sizeof(WaveFmtExtensibleBody));
			prg_exit(EXIT_FAILURE);
		}
		if (memcmp(fe->guid_tag, WAV_GUID_TAG, 14) != 0) {
			error(_("wrong format tag in extensible 'fmt ' chunk"));
			prg_exit(EXIT_FAILURE);
		}
		format = TO_CPU_SHORT(fe->guid_format, big_endian);
	}

	if (format != WAV_FMT_PCM &&
	    format != WAV_FMT_IEEE_FLOAT) {
                error(_("can't play WAVE-file format 0x%04x which is not PCM or FLOAT encoded"), format);
		prg_exit(EXIT_FAILURE);
	}
	channels = TO_CPU_SHORT(f->channels, big_endian);
	if (channels < 1) {
		error(_("can't play WAVE-files with %d tracks"), channels);
		prg_exit(EXIT_FAILURE);
	}
	hwparams->channels = channels;
	printf("WaveFmtBody format:%d,channels:%d,bit_p_spl:%d,byte_p_sec:%d,byte_p_spl:%d, channels:%d,format:%d,sample_fq:%d\n"
			,f->format, f->channels, f->bit_p_spl, f->byte_p_sec, f->byte_p_spl,
			f->channels,f->format,f->sample_fq);
	switch (TO_CPU_SHORT(f->bit_p_spl, big_endian)) {
	case 8:
		if (hwparams->format != DEFAULT_FORMAT &&
		    hwparams->format != SND_PCM_FORMAT_U8)
			fprintf(stderr, _("Warning: format is changed to U8\n"));
		hwparams->format = SND_PCM_FORMAT_U8;
		break;
	case 16:
		if (big_endian)
			native_format = SND_PCM_FORMAT_S16_BE;
		else
			native_format = SND_PCM_FORMAT_S16_LE;
		if (hwparams->format != DEFAULT_FORMAT &&
		    hwparams->format != native_format)
			fprintf(stderr, _("Warning: format is changed to %s\n"),
				snd_pcm_format_name(native_format));
		hwparams->format = native_format;
		break;
	case 24:
		switch (TO_CPU_SHORT(f->byte_p_spl, big_endian) / hwparams->channels) {
		case 3:
			if (big_endian)
				native_format = SND_PCM_FORMAT_S24_3BE;
			else
				native_format = SND_PCM_FORMAT_S24_3LE;
			if (hwparams->format != DEFAULT_FORMAT &&
			    hwparams->format != native_format)
				fprintf(stderr, _("Warning: format is changed to %s\n"),
					snd_pcm_format_name(native_format));
			hwparams->format = native_format;
			break;
		case 4:
			if (big_endian)
				native_format = SND_PCM_FORMAT_S24_BE;
			else
				native_format = SND_PCM_FORMAT_S24_LE;
			if (hwparams->format != DEFAULT_FORMAT &&
			    hwparams->format != native_format)
				fprintf(stderr, _("Warning: format is changed to %s\n"),
					snd_pcm_format_name(native_format));
			hwparams->format = native_format;
			break;
		default:
			error(_(" can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)"),
			      TO_CPU_SHORT(f->bit_p_spl, big_endian),
			      TO_CPU_SHORT(f->byte_p_spl, big_endian),
			      hwparams->channels);
			prg_exit(EXIT_FAILURE);
		}
		break;
	case 32:
		if (format == WAV_FMT_PCM) {
			if (big_endian)
				native_format = SND_PCM_FORMAT_S32_BE;
			else
				native_format = SND_PCM_FORMAT_S32_LE;
                        hwparams->format = native_format;
		} else if (format == WAV_FMT_IEEE_FLOAT) {
			if (big_endian)
				native_format = SND_PCM_FORMAT_FLOAT_BE;
			else
				native_format = SND_PCM_FORMAT_FLOAT_LE;
			hwparams->format = native_format;
		}
		break;
	default:
		error(_(" can't play WAVE-files with sample %d bits wide"),
		      TO_CPU_SHORT(f->bit_p_spl, big_endian));
		prg_exit(EXIT_FAILURE);
	}
	hwparams->rate = TO_CPU_INT(f->sample_fq, big_endian);
	
	if (size > len)
		memmove(buffer, buffer + len, size - len);
	size -= len;
	
	while (1) {
		u_int type, len;

		check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
		test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
		c = (WaveChunkHeader*)buffer;
		type = c->type;
		len = TO_CPU_INT(c->length, big_endian);
		if (size > sizeof(WaveChunkHeader))
			memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
		size -= sizeof(WaveChunkHeader);
		if (type == WAV_DATA) {
			printf("data len:%d\n", c->length);
			if (len < pbrec_count && len < 0x7ffffffe)
				pbrec_count = len;
			if (size > 0)
				memcpy(_buffer, buffer, size);
			free(buffer);
			return size;
		}
		len += len % 2;
		check_wavefile_space(buffer, len, blimit);
		test_wavefile_read(fd, buffer, &size, len, __LINE__);
		if (size > len)
			memmove(buffer, buffer + len, size - len);
		size -= len;
	}

	/* shouldn't be reached */
	return -1;
}

static u_char *speed_buf;
int slow_frame(snd_pcm_t *handle, u_char *_buffer, size_t size, int speed)
{
	int i, rc;
	u_char val;
	if (!speed_buf) {
		speed_buf = malloc(speed*size*2);
		if (!speed_buf) {
			fprintf(stderr, " mem error\n");
			return -1;
		}
	}

	switch (speed) {
		case 1:
			memcpy(speed_buf, _buffer, size);
			break;
		case 2:
			for (i=0; i<size; i++) {
				speed_buf[i*2] = _buffer[i];
				speed_buf[i*2+1] = (_buffer[i] + _buffer[i+1])/2;
				//speed_buf[i*2+1] = _buffer[i];
			}
			break;
		case 4:
			break;
		case 10:
			break;
		case 30:
			break;
		default:
			fprintf(stderr, "speed mode can't support\n");
			return 0;

	}

#if 1
	memcpy(speed_buf, _buffer, size);
	rc = snd_pcm_writei(handle, speed_buf, size);
	//rc = snd_pcm_writei(handle, _buffer, size);
	//rc = snd_pcm_writei(handle, speed_buf+size, size);
#else
	rc = snd_pcm_writei(handle, _buffer, size);
	printf("rc %d\n", rc);
	memset(speed_buf,0, speed * size);

	for (i = 1; i < speed; i++) {
		rc = snd_pcm_writei(handle, speed_buf, size);
		//rc = snd_pcm_writei(handle, _buffer, size);
		if (rc == -EPIPE) {
		  /* EPIPE means underrun */
		  fprintf(stderr, "underrun occurred\n");
		  snd_pcm_prepare(handle);
		} else if (rc < 0) {
		  fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
		}  else if (rc != (int)size) {
		  fprintf(stderr, "short write, write %d frames\n", rc);
		}
	}
#endif

	//if (speed_buf)
	//	free(speed_buf);

	return size;
}

static void wave_play(char *name)
{
	long loops;
	int rc;
	int size;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	unsigned int val;
	int dir, l, c;
	snd_pcm_uframes_t frames;
	char *buffer;
	int fd = -1;
	size_t dta;
	ssize_t dtawave;
	u_char *audiobuf = NULL;
	struct audio_params hwparams;

	if ((fd = open(name, O_RDONLY, 0)) == -1) {
		return ;
	}

	audiobuf = (u_char *)malloc(1024*10);
	if (audiobuf == NULL) {
		error(_("not enough memory"));
		return;
	}

	dta = sizeof(AuHeader);
	if ((size_t)safe_read(fd, audiobuf, dta) != dta) {
		error(_("read error"));
		prg_exit(EXIT_FAILURE);
	}

	dta = sizeof(VocHeader);
	if ((size_t)safe_read(fd, audiobuf + sizeof(AuHeader),
		dta - sizeof(AuHeader)) != dta - sizeof(AuHeader)) {
		error(_("read error"));
		prg_exit(EXIT_FAILURE);
	}

	if ((dtawave = wavefile(fd, audiobuf, dta, &hwparams)) < 0) {
		printf("wave file err\n");
		return;
	}

	/* Open PCM device for playback. */
	rc = snd_pcm_open(&handle, "default",
					SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) {
		fprintf(stderr,
				"unable to open pcm device: %s\n",
				snd_strerror(rc));
		exit(1);
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handle, params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	snd_pcm_hw_params_set_access(handle, params,
					  SND_PCM_ACCESS_RW_INTERLEAVED);

	/* Signed 16-bit little-endian format */
	snd_pcm_hw_params_set_format(handle, params, hwparams.format);

	/* Two channels (stereo) */
	snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);

	snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);

	{
		unsigned period_time = 0;
		unsigned buffer_time = 0;
		snd_pcm_uframes_t chunk_size = 0;
		snd_pcm_uframes_t buffer_size;
		snd_pcm_uframes_t period_frames = 0;
		snd_pcm_uframes_t buffer_frames = 0;
		size_t write_cnt =0;

		snd_pcm_hw_params_get_buffer_time_max(params, &buffer_time, 0);
		if (buffer_time > 500000)
			buffer_time = 500000;

		period_frames = 0;
		//period_time = 125000;
		period_time = 250000;

		snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
		snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, 0);
		snd_pcm_hw_params(handle, params);

		snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
	    snd_pcm_hw_params_get_buffer_size(params, &buffer_size);

		printf("chunk_size:%ld buffer_size:%ld\n", chunk_size, buffer_size);
		//rc = snd_pcm_hw_params(handle, params);
		if (rc < 0) {
			fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
			exit(1);
		}

	    bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
		bits_per_frame = bits_per_sample * hwparams.channels;
		chunk_bytes = chunk_size * bits_per_frame / 8;
		alam_info_dump(handle, params);
	
		buffer = realloc(audiobuf, chunk_bytes);
		while (write_cnt < pbrec_count) {
			c = pbrec_count - write_cnt;
			if (c > chunk_bytes)
				c = chunk_bytes;

			//rc = read(fd, buffer, chunk_size);
			rc = read(fd, buffer, chunk_bytes);
			if (rc == 0) {
				fprintf(stderr, "end of file on input\n");
				break;
			} else if (rc != chunk_bytes) {
				fprintf(stderr, "short read: read %d bytes\n", rc);
			}

			l = c * 8 / bits_per_frame;
			rc = slow_frame(handle, buffer, l, 2);
			write_cnt += rc*bits_per_frame/8;
		}
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);	
}

int main(int argc, char *argv[])
{

#if 0
	if (argc >=2 ) {
		printf("arg1 %s\n", argv[1]);
		read_wave(argv[1]);
	}
#endif
	//wave_play("/usr/share/sounds/alsa/Front_Left.wav");
	//wave_play("/home/wrsadmin/github/libsme/module/audio/aaaa.wav");
	wave_play("/home/wrsadmin/github/libsme/module/audio/bbc.wav");
	return 0;
}
