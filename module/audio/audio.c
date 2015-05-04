#include "audio.h"
#include <alsa/asoundlib.h>
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
#include <assert.h>
#include <termios.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>
#include "formats.h"

static char *command;
static snd_pcm_t *handle;
struct audio_params hwparams, rhwparams;

static const struct fmt_capture {
void (*start) (int fd, size_t count);
void (*end) (int fd);
char *what;
long long max_filesize;
} fmt_rec_table[] = {
//	{   NULL,       NULL,       N_("raw data"),     LLONG_MAX },
//	{   begin_voc,  end_voc,    N_("VOC"),      16000000LL },
	/* FIXME: can WAV handle exactly 2GB or less than it? */
//	{   begin_wave, end_wave,   N_("WAVE"),     2147483648LL },
//	{   begin_au,   end_au,     N_("Sparc Audio"),  LLONG_MAX }
};


static snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writei_func)(snd_pcm_t *handle, const void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*readn_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writen_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);

static int timelimit = 0;
static int quiet_mode = 0;
static int file_type = FORMAT_DEFAULT;
static int open_mode = 0;
static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
static int mmap_flag = 0;
static int interleaved = 1;
static int nonblock = 0;
static int in_aborting = 0;
static u_char *audiobuf = NULL;
static snd_pcm_uframes_t chunk_size = 0;
static unsigned period_time = 0;
static unsigned buffer_time = 0;
static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static int avail_min = -1;
static int start_delay = 0;
static int stop_delay = 0;
static int monotonic = 0;
static int interactive = 0;
static int can_pause = 0;
static int fatal_errors = 0;
static int verbose = 0;
static int vumeter = VUMETER_NONE;
static int buffer_pos = 0;
static size_t bits_per_sample, bits_per_frame;
static size_t chunk_bytes;
static int test_position = 0;
static int test_coef = 8;
static int test_nowait = 0;
static snd_output_t *log;
static long long max_file_size = 0;
static int max_file_time = 0;
static int use_strftime = 0;
volatile static int recycle_capture_file = 0;
static long term_c_lflag = -1;
static int dump_hw_params = 0;
static long long pbrec_count = LLONG_MAX, fdcount;


#define setup_chmap()	0
#define remap_data(data, count)		(data)
#define remap_datav(data, count)	(data)
/*
 * Test, if it is a .VOC file and return >=0 if ok (this is the length of rest)
 *                                       < 0 if not 
 */
int test_vocfile(void *buffer)
{
	int vocminor, vocmajor;
	VocHeader *vp = buffer;

	if (!memcmp(vp->magic, VOC_MAGIC_STRING, 20)) {
		vocminor = LE_SHORT(vp->version) & 0xFF;
		vocmajor = LE_SHORT(vp->version) / 256;
		if (LE_SHORT(vp->version) != (0x1233 - LE_SHORT(vp->coded_ver)))
			return -2;	/* coded version mismatch */
		return LE_SHORT(vp->headerlen) - sizeof(VocHeader);	/* 0 mostly */
	}
	return -1;		/* magic string fail */
}
/*
 * Safe read (for pipes)
 */
ssize_t safe_read(int fd, void *buf, size_t count)
{
	ssize_t result = 0, res;

	while (count > 0) {
		if ((res = read(fd, buf, count)) == 0)
			break;
		if (res < 0)
			return result > 0 ? result : res;
		count -= res;
		result += res;
		buf = (char *)buf + res;
	}
	return result;
}

static void done_stdin(void)
{
#if 0
    struct termios term;
    
    if (!interactive)
        return;
    if (fd == fileno(stdin) || term_c_lflag == -1)
        return;
    tcgetattr(fileno(stdin), &term);
    term.c_lflag = term_c_lflag;
    tcsetattr(fileno(stdin), TCSANOW, &term);
#endif
}
/*
 *  Subroutine to clean up before exit.
 */
void prg_exit(int code) 
{
#if 0
    done_stdin();
    if (handle)
        snd_pcm_close(handle);
    if (pidfile_written)
        remove (pidfile_name);
    exit(code);
#else
    exit(code);
#endif
}

static void header(int rtype, char *name)
{
	if (!quiet_mode) {
		if (! name)
			name = (stream == SND_PCM_STREAM_PLAYBACK) ? "stdout" : "stdin";
		
		/*
		fprintf(stderr, "%s %s '%s' : ",
			(stream == SND_PCM_STREAM_PLAYBACK) ? _("Playing") : _("Recording"),
			gettext(fmt_rec_table[rtype].what),
			name);
			*/
		
		fprintf(stderr, "%s, ", snd_pcm_format_description(hwparams.format));
		fprintf(stderr, _("Rate %d Hz, "), hwparams.rate);
		if (hwparams.channels == 1)
			fprintf(stderr, _("Mono"));
		else if (hwparams.channels == 2)
			fprintf(stderr, _("Stereo"));
		else
			fprintf(stderr, _("Channels %i"), hwparams.channels);
		fprintf(stderr, "\n");
	}
}

/*
 *  write function
 */

static ssize_t pcm_write(u_char *data, size_t count)
{
	ssize_t r;
	ssize_t result = 0;
	 printf("%p count:%d\n", data, count);

	if (count < chunk_size) {
		snd_pcm_format_set_silence(hwparams.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwparams.channels);
		count = chunk_size;
	}
	while (count > 0 && !in_aborting) {
	//	r = writei_func(handle, data, count);
		r = snd_pcm_writei(handle, data, count);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
				snd_pcm_wait(handle, 100);
		} else if (r == -EPIPE) {
			//xrun();
		} else if (r == -ESTRPIPE) {
			//suspend();
		} else if (r < 0) {
			error(_("write error: %s"), snd_strerror(r));
			prg_exit(EXIT_FAILURE);
		}
		if (r > 0) {
			//if (vumeter)
			//		compute_max_peak(data, r * hwparams.channels);
			result += r;
			count -= r;
			data += r * bits_per_frame / 8;
		}
	}
	return result;
}

static void set_params(void)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;
	int err;
	size_t n;
	unsigned int rate;
	snd_pcm_uframes_t start_threshold, stop_threshold;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		error(_("Broken configuration for this PCM: no configurations available"));
		prg_exit(EXIT_FAILURE);
	}
	if (dump_hw_params) {
		fprintf(stderr, _("HW Params of device \"%s\":\n"),
			snd_pcm_name(handle));
		fprintf(stderr, "--------------------\n");
		snd_pcm_hw_params_dump(params, log);
		fprintf(stderr, "--------------------\n");
	}
	if (mmap_flag) {
		snd_pcm_access_mask_t *mask = alloca(snd_pcm_access_mask_sizeof());
		snd_pcm_access_mask_none(mask);
		snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
		snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
		snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_COMPLEX);
		err = snd_pcm_hw_params_set_access_mask(handle, params, mask);
	} else if (interleaved)
		err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_INTERLEAVED);
	else
		err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_NONINTERLEAVED);
	if (err < 0) {
		error(_("Access type not available"));
		prg_exit(EXIT_FAILURE);
	}
	err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
	if (err < 0) {
		error(_("Sample format non available"));
		//show_available_sample_formats(params);
		prg_exit(EXIT_FAILURE);
	}
	err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
	if (err < 0) {
		error(_("Channels count non available"));
		prg_exit(EXIT_FAILURE);
	}

#if 0
	err = snd_pcm_hw_params_set_periods_min(handle, params, 2);
	assert(err >= 0);
#endif
	rate = hwparams.rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);
	assert(err >= 0);
	if ((float)rate * 1.05 < hwparams.rate || (float)rate * 0.95 > hwparams.rate) {
		if (!quiet_mode) {
			char plugex[64];
			const char *pcmname = snd_pcm_name(handle);
			fprintf(stderr, _("Warning: rate is not accurate (requested = %iHz, got = %iHz)\n"), rate, hwparams.rate);
			if (! pcmname || strchr(snd_pcm_name(handle), ':'))
				*plugex = 0;
			else
				snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
					 snd_pcm_name(handle));
			fprintf(stderr, _("         please, try the plug plugin %s\n"),
				plugex);
		}
	}
	rate = hwparams.rate;
	if (buffer_time == 0 && buffer_frames == 0) {
		err = snd_pcm_hw_params_get_buffer_time_max(params,
							    &buffer_time, 0);
		assert(err >= 0);
		if (buffer_time > 500000)
			buffer_time = 500000;
	}
	if (period_time == 0 && period_frames == 0) {
		if (buffer_time > 0)
			period_time = buffer_time / 4;
		else
			period_frames = buffer_frames / 4;
	}
	if (period_time > 0)
		err = snd_pcm_hw_params_set_period_time_near(handle, params,
							     &period_time, 0);
	else
		err = snd_pcm_hw_params_set_period_size_near(handle, params,
							     &period_frames, 0);
	assert(err >= 0);
	if (buffer_time > 0) {
		err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
							     &buffer_time, 0);
	} else {
		err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
							     &buffer_frames);
	}
	assert(err >= 0);
	monotonic = snd_pcm_hw_params_is_monotonic(params);
	can_pause = snd_pcm_hw_params_can_pause(params);
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		error(_("Unable to install hw params:"));
		snd_pcm_hw_params_dump(params, log);
		prg_exit(EXIT_FAILURE);
	}
	snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (chunk_size == buffer_size) {
		error(_("Can't use period equal to buffer size (%lu == %lu)"),
		      chunk_size, buffer_size);
		prg_exit(EXIT_FAILURE);
	}
	snd_pcm_sw_params_current(handle, swparams);
	if (avail_min < 0)
		n = chunk_size;
	else
		n = (double) rate * avail_min / 1000000;
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

	/* round up to closest transfer boundary */
	n = buffer_size;
	if (start_delay <= 0) {
		start_threshold = n + (double) rate * start_delay / 1000000;
	} else
		start_threshold = (double) rate * start_delay / 1000000;
	if (start_threshold < 1)
		start_threshold = 1;
	if (start_threshold > n)
		start_threshold = n;
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
	assert(err >= 0);
	if (stop_delay <= 0) 
		stop_threshold = buffer_size + (double) rate * stop_delay / 1000000;
	else
		stop_threshold = (double) rate * stop_delay / 1000000;
	err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
	assert(err >= 0);

	if (snd_pcm_sw_params(handle, swparams) < 0) {
		error(_("unable to install sw params:"));
		snd_pcm_sw_params_dump(swparams, log);
		prg_exit(EXIT_FAILURE);
	}

	if (setup_chmap())
		prg_exit(EXIT_FAILURE);

	if (verbose)
		snd_pcm_dump(handle, log);

	bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
	bits_per_frame = bits_per_sample * hwparams.channels;
	chunk_bytes = chunk_size * bits_per_frame / 8;
	audiobuf = realloc(audiobuf, chunk_bytes);
	if (audiobuf == NULL) {
		error(_("not enough memory"));
		prg_exit(EXIT_FAILURE);
	}
	// fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);

	/* stereo VU-meter isn't always available... */
	if (vumeter == VUMETER_STEREO) {
		if (hwparams.channels != 2 || !interleaved || verbose > 2)
			vumeter = VUMETER_MONO;
	}

	/* show mmap buffer arragment */
	if (mmap_flag && verbose) {
		const snd_pcm_channel_area_t *areas;
		snd_pcm_uframes_t offset, size = chunk_size;
		int i;
		err = snd_pcm_mmap_begin(handle, &areas, &offset, &size);
		if (err < 0) {
			error(_("snd_pcm_mmap_begin problem: %s"), snd_strerror(err));
			prg_exit(EXIT_FAILURE);
		}
		for (i = 0; i < hwparams.channels; i++)
			fprintf(stderr, "mmap_area[%i] = %p,%u,%u (%u)\n", i, areas[i].addr, areas[i].first, areas[i].step, snd_pcm_format_physical_width(hwparams.format));
		/* not required, but for sure */
		snd_pcm_mmap_commit(handle, offset, 0);
	}

	buffer_frames = buffer_size;	/* for position test */
}

/* playing raw data */
void playback_go(int fd, size_t loaded, long  long count, int rtype, u_char *audiobuf, char *name )
{
    int l, r;
    long long written = 0; 
    long long c;

    header(rtype, name);
    set_params();

    while (loaded > chunk_bytes && written < count && !in_aborting) {
        if (pcm_write(audiobuf + written, chunk_size) <= 0)
            return;
        written += chunk_bytes;
        loaded -= chunk_bytes;
    }    
    if (written > 0 && loaded > 0) 
        memmove(audiobuf, audiobuf + written, loaded);

    l = loaded;
    while (written < count && !in_aborting) {
        do {
            c = count - written;
            if (c > chunk_bytes)
                c = chunk_bytes;
            c -= l;

            if (c == 0)
                break;
            r = safe_read(fd, audiobuf + l, c);
            if (r < 0) { 
                perror(name);
                prg_exit(EXIT_FAILURE);
            }
            fdcount += r;
            if (r == 0)
                break;
            l += r;
        } while ((size_t)l < chunk_bytes);
        l = l * 8 / bits_per_frame;
        r = pcm_write(audiobuf, l);
        if (r != l)
            break;
        r = r * bits_per_frame / 8; 
        written += r;
        l = 0; 
    }    
    snd_pcm_nonblock(handle, 0);
    snd_pcm_drain(handle);
	snd_pcm_nonblock(handle, nonblock);
}

#define DEFAULT_FORMAT      SND_PCM_FORMAT_U8
#define DEFAULT_SPEED       8000
/* calculate the data count to read from/to dsp */
static long long calc_count(void)
{
	long long count;

	if (timelimit == 0) {
		count = pbrec_count;
	} else {
		count = snd_pcm_format_size(hwparams.format, hwparams.rate * hwparams.channels);
		count *= (long long )timelimit;
	}
	return count < pbrec_count ? count : pbrec_count;
}

void pcm_init(void)
{
	int err;
	char *pcm_name = "default";

	snd_pcm_info_t *info;
	snd_pcm_info_alloca(&info);
	stream = SND_PCM_STREAM_PLAYBACK;
	rhwparams.format = DEFAULT_FORMAT;
	rhwparams.rate = DEFAULT_SPEED;
	rhwparams.channels = 1;
	err = snd_pcm_open(&handle, pcm_name, stream, open_mode);
	if (err < 0) {
		printf(" snd open failed\n");
	return;
	}

	if ((err = snd_pcm_info(handle, info)) < 0) {
	}

	hwparams = rhwparams;
	chunk_size = 1024;
	writei_func = snd_pcm_writei;
	readi_func = snd_pcm_readi;
	writen_func = snd_pcm_writen;
	readn_func = snd_pcm_readn;
	pbrec_count = LLONG_MAX;

}

void playback(char *name)
{
	int fd = -1;
	int ofs;
	size_t dta;
	ssize_t dtawave;

	pcm_init();

	audiobuf = (u_char *)malloc(1024);
	printf("%p \n", audiobuf);
	if (audiobuf == NULL) {
		error(_("not enough memory"));
		return;
	}

	if ((fd = open(name, O_RDONLY, 0)) == -1) {
		return ;
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
		prg_exit(EXIT_FAILURE);;
	}

	if ((dtawave = test_wavefile(fd, audiobuf, dta,&hwparams)) >= 0) {
	 	//pbrec_count = calc_count();

		pbrec_count = 137090;
		playback_go(fd, dta, pbrec_count, FORMAT_WAVE, audiobuf, name);
	}
		
}