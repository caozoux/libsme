#include <alsa/asoundlib.h>
#include "audio.h"
/*information as follow:
ALSA library version: 1.0.27.2

PCM stream types:
 PLAYBACK
 CAPTURE

PCM access types:
 MMAP_INTERLEAVED
 MMAP_NONINTERLEAVED
 MMAP_COMPLEX
 RW_INTERLEAVED
 RW_NONINTERLEAVED

PCM formats:
 S8 (Signed 8 bit)
 U8 (Unsigned 8 bit)
 S16_LE (Signed 16 bit Little Endian)
 S16_BE (Signed 16 bit Big Endian)
 U16_LE (Unsigned 16 bit Little Endian)
 U16_BE (Unsigned 16 bit Big Endian)
 S24_LE (Signed 24 bit Little Endian)
 S24_BE (Signed 24 bit Big Endian)
 U24_LE (Unsigned 24 bit Little Endian)
 U24_BE (Unsigned 24 bit Big Endian)
 S32_LE (Signed 32 bit Little Endian)
 S32_BE (Signed 32 bit Big Endian)
 U32_LE (Unsigned 32 bit Little Endian)
 U32_BE (Unsigned 32 bit Big Endian)
 FLOAT_LE (Float 32 bit Little Endian)
 FLOAT_BE (Float 32 bit Big Endian)
 FLOAT64_LE (Float 64 bit Little Endian)
 FLOAT64_BE (Float 64 bit Big Endian)
 IEC958_SUBFRAME_LE (IEC-958 Little Endian)
 IEC958_SUBFRAME_BE (IEC-958 Big Endian)
 MU_LAW (Mu-Law)
 A_LAW (A-Law)
 IMA_ADPCM (Ima-ADPCM)
 MPEG (MPEG)
 GSM (GSM)
 SPECIAL (Special)
 S24_3LE (Signed 24 bit Little Endian in 3bytes)
 S24_3BE (Signed 24 bit Big Endian in 3bytes)
 U24_3LE (Unsigned 24 bit Little Endian in 3bytes)
 U24_3BE (Unsigned 24 bit Big Endian in 3bytes)
 S20_3LE (Signed 20 bit Little Endian in 3bytes)
 S20_3BE (Signed 20 bit Big Endian in 3bytes)
 U20_3LE (Unsigned 20 bit Little Endian in 3bytes)
 U20_3BE (Unsigned 20 bit Big Endian in 3bytes)
 S18_3LE (Signed 18 bit Little Endian in 3bytes)
 S18_3BE (Signed 18 bit Big Endian in 3bytes)
 U18_3LE (Unsigned 18 bit Little Endian in 3bytes)
 U18_3BE (Unsigned 18 bit Big Endian in 3bytes)
 G723_24 (G.723 (ADPCM) 24 kbit/s, 8 samples in 3 bytes)
 G723_24_1B (G.723 (ADPCM) 24 kbit/s, 1 sample in 1 byte)
 G723_40 (G.723 (ADPCM) 40 kbit/s, 8 samples in 3 bytes)
 G723_40_1B (G.723 (ADPCM) 40 kbit/s, 1 sample in 1 byte)
 DSD_U8 (Direct Stream Digital, 1-byte (x8), oldest bit in MSB)
 DSD_U16_LE (Direct Stream Digital, 2-byte (x16), little endian, oldest bits in MSB)

PCM subformats:
 STD (Standard)

PCM states:
 OPEN
 SETUP
 PREPARED
 RUNNING
 XRUN
 DRAINING
 PAUSED
 SUSPENDED
 DISCONNECTED
*/
int alas_info(void)
{
	int val;

	printf("ALSA library version: %s\n",
			  SND_LIB_VERSION_STR);

	printf("\nPCM stream types:\n");
	for (val = 0; val <= SND_PCM_STREAM_LAST; val++)
		printf(" %s\n",
		  snd_pcm_stream_name((snd_pcm_stream_t)val));

	printf("\nPCM access types:\n");
	for (val = 0; val <= SND_PCM_ACCESS_LAST; val++)
		printf(" %s\n",
		  snd_pcm_access_name((snd_pcm_access_t)val));

	printf("\nPCM formats:\n");
	for (val = 0; val <= SND_PCM_FORMAT_LAST; val++)
		if (snd_pcm_format_name((snd_pcm_format_t)val)
		  != NULL)
		  printf(" %s (%s)\n",
			snd_pcm_format_name((snd_pcm_format_t)val),
			snd_pcm_format_description(
							   (snd_pcm_format_t)val));

	printf("\nPCM subformats:\n");
	for (val = 0; val <= SND_PCM_SUBFORMAT_LAST;
		   val++)
		printf(" %s (%s)\n",
		  snd_pcm_subformat_name((
			snd_pcm_subformat_t)val),
		  snd_pcm_subformat_description((
			snd_pcm_subformat_t)val));

	printf("\nPCM states:\n");
	for (val = 0; val <= SND_PCM_STATE_LAST; val++)
		printf(" %s\n",
			   snd_pcm_state_name((snd_pcm_state_t)val));

	return 0;
}


int alam_info_dump(snd_pcm_t *dev_handle, snd_pcm_hw_params_t *params)
{
	int rc;
	snd_pcm_t *handle;
	unsigned int val, val2;
	int dir;
	snd_pcm_uframes_t frames;

	handle = dev_handle;

	/* Display information about the PCM interface */

	printf("PCM handle name = '%s'\n", snd_pcm_name(handle));

	printf("PCM state = %s\n", snd_pcm_state_name(snd_pcm_state(handle)));

	snd_pcm_hw_params_get_access(params, (snd_pcm_access_t *) &val);
	printf("access type = %s\n", snd_pcm_access_name((snd_pcm_access_t)val));

	snd_pcm_hw_params_get_format(params,(snd_pcm_format_t *) &val);
	printf("format = '%s' (%s)\n", snd_pcm_format_name((snd_pcm_format_t)val),
				    snd_pcm_format_description((snd_pcm_format_t)val));

	snd_pcm_hw_params_get_subformat(params, (snd_pcm_subformat_t *)&val);
	printf("subformat = '%s' (%s)\n", snd_pcm_subformat_name((snd_pcm_subformat_t)val),
				    snd_pcm_subformat_description((snd_pcm_subformat_t)val));

	snd_pcm_hw_params_get_channels(params, &val);
	printf("channels = %d\n", val);

	snd_pcm_hw_params_get_rate(params, &val, &dir);
	printf("rate = %d bps\n", val);

	snd_pcm_hw_params_get_period_time(params,
			                                    &val, &dir);
	printf("period time = %d us\n", val);

	snd_pcm_hw_params_get_period_size(params,
			                                    &frames, &dir);
	printf("period size = %d frames\n", (int)frames);

	snd_pcm_hw_params_get_buffer_time(params,
			                                    &val, &dir);
	printf("buffer time = %d us\n", val);

	snd_pcm_hw_params_get_buffer_size(params, (snd_pcm_uframes_t *) &val);
	printf("buffer size = %d frames\n", val);

	snd_pcm_hw_params_get_periods(params, &val, &dir);
	printf("periods per buffer = %d frames\n", val);

	snd_pcm_hw_params_get_rate_numden(params, &val, &val2);
	printf("exact rate = %d/%d bps\n", val, val2);

	val = snd_pcm_hw_params_get_sbits(params);
	printf("significant bits = %d\n", val);

	snd_pcm_hw_params_get_tick_time(params, &val, &dir);
	printf("tick time = %d us\n", val);

	val = snd_pcm_hw_params_is_batch(params);
	printf("is batch = %d\n", val);

	val = snd_pcm_hw_params_is_block_transfer(params);
	printf("is block transfer = %d\n", val);

	val = snd_pcm_hw_params_is_double(params);
	printf("is double = %d\n", val);

	val = snd_pcm_hw_params_is_half_duplex(params);
	printf("is half duplex = %d\n", val);

	val = snd_pcm_hw_params_is_joint_duplex(params);
	printf("is joint duplex = %d\n", val);

	val = snd_pcm_hw_params_can_overrange(params);
	printf("can overrange = %d\n", val);

	val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
	printf("can mmap = %d\n", val);

	val = snd_pcm_hw_params_can_pause(params);
	printf("can pause = %d\n", val);

	val = snd_pcm_hw_params_can_resume(params);
	printf("can resume = %d\n", val);

	val = snd_pcm_hw_params_can_sync_start(params);
	printf("can sync start = %d\n", val);

	return 0;
}

/* read from stdin, play it*/
int alas_stdin_play(void)
{
  long loops;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  char *buffer;

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
                                    &val, &dir);
  /* 5 seconds in microseconds divided by
   * period time */
  loops = 5000000 / val;

  while (loops > 0) {
		loops--;
		rc = read(0, buffer, size);
		if (rc == 0) {
			fprintf(stderr, "end of file on input\n");
			break;
		} else if (rc != size) {
			fprintf(stderr, "short read: read %d bytes\n", rc);
		}
		rc = snd_pcm_writei(handle, buffer, frames);
		if (rc == -EPIPE) {
			/* EPIPE means underrun */
			fprintf(stderr, "underrun occurred\n");
			snd_pcm_prepare(handle);
		} else if (rc < 0) {
			fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
		}  else if (rc != (int)frames) {
			fprintf(stderr, "short write, write %d frames\n", rc);
		}
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);

  return 0;
}
