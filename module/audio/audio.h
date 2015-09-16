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

struct audio_params {
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int rate;
};

ssize_t safe_read(int fd, void *buf, size_t count);
//ssize_t test_wavefile(int fd, u_char *_buffer, size_t size,struct audio_params *hwparams);
ssize_t test_wavefile(int fd, u_char *_buffer, size_t size);
void playback_go(int fd, size_t loaded, long  long count, int rtype, u_char *audiobuf, char *name);
void playback(char *name);
void pcm_init(void);
void prg_exit(int code);
int alam_info_dump(snd_pcm_t *dev_handle, snd_pcm_hw_params_t *params);

size_t test_wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize, int line);
#define check_wavefile_space(buffer, len, blimit) \
	if (len > blimit) { \
		blimit = len; \
		if ((buffer = realloc(buffer, blimit)) == NULL) { \
			error(_("not enough memory"));		  \
			prg_exit(EXIT_FAILURE);  \
		} \
	}
#endif
