#include "audio.h"
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include <termios.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>
#include "audio.h"
#include "libmetype.h"

#define DEFAULT_FORMAT		SND_PCM_FORMAT_U8
#define DEFAULT_SPEED 		8000


extern rhwparams,hwparams;
//static off64_t pbrec_count = LLONG_MAX, fdcount;
static long long pbrec_count = LLONG_MAX, fdcount;
/*
 * helper for test_wavefile
 */

static size_t test_wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize, int line)
{
	if (*size >= reqsize)
		return *size;
	if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
		error(_("read error (called from line %i)"), line);
		prg_exit(EXIT_FAILURE);
	}
	return *size = reqsize;
}

#define check_wavefile_space(buffer, len, blimit) \
	if (len > blimit) { \
		blimit = len; \
		if ((buffer = realloc(buffer, blimit)) == NULL) { \
			error(_("not enough memory"));		  \
			prg_exit(EXIT_FAILURE);  \
		} \
	}


struct WaveHeaderM {
	/*
		big 	ChunkID 	4 	文件头标识，一般就是" RIFF" 四个字母
		little 	ChunkSize 	4 	整个数据文件的大小，不包括上面ID和Size本身
		big 	Format 	4 	一般就是" WAVE" 四个字母
		big 	SubChunk1ID 	4 	格式说明块，本字段一般就是"fmt "
		little 	SubChunk1Size 	4 	本数据块的大小，不包括ID和Size字段本身
		little 	AudioFormat 	2 	音频的格式说明
		little 	NumChannels 	2 	声道数
		little 	SampleRate 	4 	采样率
		little 	ByteRate 	4 	比特率，每秒所需要的字节数
		little 	BlockAlign 	2 	数据块对齐单元
		little 	BitsPerSample 	2 	采样时模数转换的分辨率
		big 	SubChunk2ID 	4 	真正的声音数据块，本字段一般是"data"
		little 	SubChunk2Size 	4 	本数据块的大小，不包括ID和Size字段本身
		little 	Data 	N 	音频的采样数据
		*/
	u32 ChunkID;// 	4 	文件头标识，一般就是" RIFF" 四个字母
	u32 ChunkSize;// 	4 	整个数据文件的大小，不包括上面ID和Size本身
	u32 Format;//	4 	一般就是" WAVE" 四个字母
	u32 SubChunk1ID;//	4 	格式说明块，本字段一般就是"fmt "
	u32 SubChunk1Size;//	4 	本数据块的大小，不包括ID和Size字段本身
	u16 AudioFormat;//	2 	音频的格式说明
	u16 NumChannels;//	2 	声道数
	u32 SampleRate;//	4 	采样率
	u32 ByteRate;//	4 	比特率，每秒所需要的字节数
	u16 BlockAlign;//	2 	数据块对齐单元
	u16 BitsPerSample;//	2 	采样时模数转换的分辨率
	u32 SubChunk2ID;//	4 	真正的声音数据块，本字段一般是"data"
	u32 SubChunk2Size;//	4 	本数据块的大小，不包括ID和Size字段本身
//little 	Data 	N 	音频的采样数据
};

void wavehead_dump(void * pointer)
{
	struct WaveHeaderM *wavehead;
	wavehead= (struct WaveHeaderM *) pointer;

	//printf("ChunkID:%08x\n",wavehead->ChunkID);
	if (wavehead->ChunkID == WAV_RIFF) 
		printf("ChunkID: RIFF\n");

	printf("ChunkSize:%d\n",wavehead->ChunkSize);

	//printf("Format:%08x\n",wavehead->Format);
	if (wavehead->Format == WAV_WAVE)
		printf("Format: WAVE\n");

	//printf("SubChunk1ID:%08x\n",wavehead->SubChunk1ID);
	if (wavehead->SubChunk1ID == WAV_FMT)
		printf("SubChunk1ID: WAV_FMT\n");

	printf("SubChunk1Size:%08x\n",wavehead->SubChunk1Size);
	printf("AudioFormat:%08x\n",wavehead->AudioFormat);
	printf("NumChannels:%08x\n",wavehead->NumChannels);
	printf("SampleRate:%d\n",wavehead->SampleRate);
	printf("ByteRate:%d\n",wavehead->ByteRate);
	printf("BlockAlign:%08x\n",wavehead->BlockAlign);
	printf("BitsPerSample:%08x\n",wavehead->BitsPerSample);

	//printf("SubChunk2ID:%08x\n",wavehead->SubChunk2ID);
	if (wavehead->SubChunk2ID == WAV_DATA)
		printf("SubChunk2ID: WAV_DATA\n");
	printf("SubChunk2Size:%d\n",wavehead->SubChunk2Size);
}

void read_wave(char *wave_name)
{
	struct WaveHeaderM wavehead;
	ssize_t res;
	int fd;
	fd = open(wave_name, O_RDONLY);
	if (fd <= 0) {
		printf("open file error\n");
		return;
	}


    if ((res = read(fd, (void *)&wavehead, sizeof(wavehead))) == 0) {
		printf("error read \n");
		return -1;
	}

	if (wavehead->Format != WAV_WAVE || wavehead->SubChunk2ID == WAV_DATA
			|| wavehead->SubChunk1ID == WAV_FMT) {
		printf("error, it isn't wave file\n");
		goto _end;
	}


	printf("SubChunk1Size:%08x\n",wavehead->SubChunk1Size);
	printf("AudioFormat:%08x\n",wavehead->AudioFormat);
	printf("NumChannels:%08x\n",wavehead->NumChannels);
	printf("SampleRate:%d\n",wavehead->SampleRate);
	printf("ByteRate:%d\n",wavehead->ByteRate);
	printf("BlockAlign:%08x\n",wavehead->BlockAlign);
	printf("BitsPerSample:%08x\n",wavehead->BitsPerSample);


	wavehead_dump(&wavehead);
_end:
	close(fd);

}

ssize_t wavefile_init(int fd, struct audio_params *hwparams)
{
	u_char *buffer = NULL;
	struct WaveHeaderM wavehead;
	ssize_t res;

    if ((res = read(fd, (void *)&wavehead, sizeof(wavehead))) == 0) {
		printf("error read \n");
		return -1;
	}

	if (wavehead->Format != WAV_WAVE || wavehead->SubChunk2ID == WAV_DATA
			|| wavehead->SubChunk1ID == WAV_FMT) {
		printf("error, it isn't wave file\n");
		goto _end;
	}

	hwparams->channels = wavehead->NumChannels;
	hwparams->format = SND_PCM_FORMAT_S16_LE;
	hwparams->rate = wavehead->SampleRate;

	close(fd);
	return 0;

_end:
	close(fd);
	return -1;
}

int playwave(snd_pcm_t *handle, char *wavename, audio_params *hwparams)
{
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
	snd_pcm_hw_params_set_format(handle, params,
							  SND_PCM_FORMAT_S16_LE);

	/* Two channels (stereo) */
	snd_pcm_hw_params_set_channels(handle, params, 2);

	/* 44100 bits/second sampling rate (CD quality) */
	val = 44100;
	snd_pcm_hw_params_set_rate_near(handle, params,
								  &val, &dir);

	/* Set period size to 32 frames. */
	frames = 32;
	snd_pcm_hw_params_set_period_size_near(handle,
							  params, &frames, &dir);

	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0) {
		fprintf(stderr,
				"unable to set hw parameters: %s\n",
				snd_strerror(rc));
		exit(1);
	}

	/* Use a buffer large enough to hold one period */
	snd_pcm_hw_params_get_period_size(params, &frames,
									&dir);
	size = frames * 4; /* 2 bytes/sample, 2 channels */
	buffer = (char *) malloc(size);

	/* We want to loop for 5 seconds */
	snd_pcm_hw_params_get_period_time(params,
}
/*
 * test, if it's a .WAV file, > 0 if ok (and set the speed, stereo etc.)
 *                            == 0 if not
 * Value returned is bytes to be discarded.
 */
ssize_t test_wavefile(int fd, u_char *_buffer, size_t size,struct audio_params *hwparams)
{
	WaveHeader *h = (WaveHeader *)_buffer;
	u_char *buffer = NULL;
	size_t blimit = 0;
	WaveFmtBody *f;
	WaveChunkHeader *c;
	u_int type, len;
	unsigned short format, channels;
	int big_endian, native_format;
	printf("%s size:%d\n", __func__, size);

	wavefile_init(fd, _buffer, size, hwparams);
	exit(0);

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
			//fprintf(stderr, "Warning: format is changed to U8\n");
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
