#ifndef _ME_AUDIO_H_
#define _ME_AUDIO_H_
#include <sys/types.h>
#include "formats.h"
#include <alsa/asoundlib.h>

//#define _(msgid) gettext (msgid)
#define FORMAT_DEFAULT		-1
#define FORMAT_RAW		0
#define FORMAT_VOC		1
#define FORMAT_WAVE		2
#define FORMAT_AU		3

#define _(msgid) (msgid)

enum {
	VUMETER_NONE,
	VUMETER_MONO,
	VUMETER_STEREO
};

static struct audio_params{
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
};

ssize_t safe_read(int fd, void *buf, size_t count);
ssize_t test_wavefile(int fd, u_char *_buffer, size_t size,struct audio_params *hwparams);
void playback_go(int fd, size_t loaded, long  long count, int rtype, u_char *audiobuf, char *name );
void playback(char *name);
void pcm_init(void);
void prg_exit(int code); 

#endif
