#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "config.h"

#include "alsa.h"

#ifdef FFT
#include <fftw3.h>
#include "piyavsky.c"
#endif

#pragma GCC diagnostic ignored "-Wchar-subscripts"

char fnote(double f)
{
  char n = 1;

  while (n < N_HALFTONE) {
    if (fabs(f - notes[n]) > fabs(f - notes[n - 1]))
      break;

    n++;
  }

  return n - 1 + BOTTOM_HALFTONE;
}

double goertzel(double f, short *buf, unsigned int len)
{
  double s[len];
  unsigned int i;
  const double coeff = 2 * cos(coeff_const * f);

  s[0] = buf[0];
  s[1] = buf[1] + coeff * buf[0];

  for (i = 2; i < len; i++)
    s[i] = buf[i] + coeff * s[i - 1] - s[i - 2];

  s[0] = s[len - 2] * s[len - 2] + s[len - 1] * s[len - 1] - coeff * s[len - 2] * s[len - 1];

  return sqrt(fabs(s[0]));
}

int main(int argc, char **argv)
{
  int dir, rv;
  char flag = 0;
  double step, pvsky;
  unsigned int rate = RATE;
  unsigned int i;
  snd_pcm_t *h;
  snd_pcm_hw_params_t *hw_params;
#ifdef FFT
  double l;
  unsigned int j;
  fftw_plan p;
  fftw_complex *in, *out;
#endif
#ifndef FFT
  double max, tmp;
  unsigned int maxi;
#endif
#ifdef STOPWATCH
  struct timespec t0, t1;
#endif

  /* Using variable 'rv' to calculate & initialize notes array. */
  for (rv = 0; rv < N_HALFTONE; rv++)
    notes[rv] = note(BOTTOM_HALFTONE + rv);

  if ((rv = snd_pcm_open(&h, argv[1], SND_PCM_STREAM_CAPTURE, 0)) != 0) {
    fprintf(stderr, "snd_pcm_open(): %s\n", snd_strerror(rv));

    return -1;
  }

  if ((rv = snd_pcm_hw_params_malloc(&hw_params)) != 0) {
    fprintf(stderr, "snd_pcm_hw_params_malloc(): %s\n", snd_strerror(rv));

    return -1;
  }

  snd_pcm_hw_params_any(h, hw_params);

  if ((rv = snd_pcm_hw_params_set_access(h, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) != 0) {
    fprintf(stderr, "snd_pcm_hw_params_set_access(): %s\n", snd_strerror(rv));

    return -1;
  }

#ifdef BB
  if ((rv = snd_pcm_hw_params_set_format(h, hw_params, SND_PCM_FORMAT_U16_BE)) != 0) {
#endif
#ifndef BB
  if ((rv = snd_pcm_hw_params_set_format(h, hw_params, SND_PCM_FORMAT_S16_LE)) != 0) {
#endif
    fprintf(stderr, "snd_pcm_hw_params_set_format(): %s\n", snd_strerror(rv));

    return -1;
  }

  if ((rv = snd_pcm_hw_params_set_channels(h, hw_params, 2)) != 0) {
    fprintf(stderr, "snd_pcm_hw_params_set_channels(): %s\n", snd_strerror(rv));

    return -1;
  }

  if ((rv = snd_pcm_hw_params_set_rate_near(h, hw_params, &rate, &dir)) != 0) {
    fprintf(stderr, "snd_pcm_hw_params_set_rate_near(): %s\n", snd_strerror(rv));

    return -1;
  }

  printf("Frequency is set to %d Hz.\n", rate);

  if ((rv = snd_pcm_hw_params_set_period_size_near(h, hw_params, &frames, &dir)) != 0) {
    fprintf(stderr, "snd_pcm_hw_params_set_period_size_near(): %s\n", snd_strerror(rv));

    return -1;
  }

  if (frames != PERIOD_SIZE) {
    fprintf(stderr, "frames != PERIOD_SIZE\n");

    fprintf(stderr, "%li != %d\n", frames, PERIOD_SIZE);

    return -1;
  }

  printf("Period size is set to %li frames.\n", frames);

  if ((rv = snd_pcm_hw_params(h, hw_params)) != 0) {
    fprintf(stderr, "snd_pcm_hw_params(): %s\n", snd_strerror(rv));

    return -1;
  }

  step = notes[13] - notes[12];

  step /= 2;

  printf("Step is %f Hz.\n", step);

  mult = rate / step / frames;

  step = (double)(mult * frames) / rate;

  printf("* Step is %f Hz.\n", step);

  mult /= 2;
  step *= 2;

  if (mult != BUFSIZE) {
    fprintf(stderr, "mult != BUFSIZE\n");

    return -1;
  }

  printf("Buffers are multiplied by %d.\n", mult);

  if ((rv = snd_pcm_prepare(h)) != 0) {
    fprintf(stderr, "snd_pcm_prepare(): %s\n", snd_strerror(rv));

    return -1;
  }

#ifdef FFT
  in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * frames * mult);
  out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * frames * mult);

  p = fftw_plan_dft_1d(frames * mult, in, out, FFTW_FORWARD, FFTW_MEASURE);
#endif

  while (1) {
    for (i = 0; i < mult; i++)
      while ((rv = snd_pcm_readi(h, buf + frames * i, frames)) != (int)frames) {
	fprintf(stderr, "snd_pcm_readi(): %s\n", snd_strerror(rv));

	if ((rv = snd_pcm_prepare(h)) != 0) {
	  fprintf(stderr, "snd_pcm_prepare(): %s\n", snd_strerror(rv));

	  return -1;
	}
      }

    for (i = 0; i * 2 < frames * mult; i++)
      buf[i] = buf[i * 2 + 1];

#ifdef STOPWATCH
    clock_gettime(CLOCK_REALTIME, &t0);
#endif

#ifdef FFT
    for (i = 0; i < frames * mult / 2; i++)
      *in[i] = buf[i];

    for (i = 1; step * i * 2 < notes[12]; i++)
      i++;

#ifdef PIYAVSKY
    if (!flag) {
      fftw_execute(p);

      l = findl_fft(i, frames * mult, out);

      l = 10 * log10(l);

      flag = 1;
    }

    pvsky = piyavsky(l, step, step * i * 2, notes[N_HALFTONE - 1], notes[N_HALFTONE - 1] / 2);

    if (pnote(fnote(pvsky)) == -1) {
      fprintf(stderr, "pnote() returned -1\n");

      return -1;
    }
#endif
#ifndef PIYAVSKY
    fftw_execute(p);

    for (j = 0; step * i * 2 < notes[N_HALFTONE - 1]; i++) {
      if (*out[i] < 0)
	*out[i] = 0 - *out[i];

      if (*out[i] > *out[j])
	j = i;
    }

    if (pnote(fnote(step * j * 2)) == -1) {
      fprintf(stderr, "pnote() returned -1\n");

      return -1;
    }
#endif
#endif
#ifndef FFT
    for (i = 0, maxi = 0; i < N_HALFTONE; i++) {
      tmp = goertzel(notes[i], buf, frames * mult / 2);
      tmp = 10 * log10(tmp);

      if (i == 0) {
	max = tmp;

	continue;
      }

      if (tmp > max) {
	max = tmp;
	maxi = i;
      }
    }

    if (pnote(fnote(notes[maxi])) == -1) {
      fprintf(stderr, "pnote() returned -1\n");

      return -1;
    }
#endif

#ifdef STOPWATCH
    clock_gettime(CLOCK_REALTIME, &t1);

    printf(" %li.", t1.tv_sec - t0.tv_sec);
    if (t1.tv_sec - t0.tv_sec)
      printf("%09li second(s).\n", 1000000000 - t0.tv_nsec + t1.tv_nsec);
    else
      printf("%09li second(s).\n", t1.tv_nsec - t0.tv_nsec);
#endif
  }

  snd_pcm_drain(h);
  snd_pcm_close(h);

#ifdef FFT
  fftw_destroy_plan(p);
  fftw_free(in);
  fftw_free(out);
#endif

  return 0;
}


double note(double n)
{
  /* A4 = 440 Hz. */
  return 440 * pow(pow(2, pow_const), n);
}

int pnote(char n)
{
  if (n < BOTTOM_HALFTONE) {
    fprintf(stderr, "n < -(48 + 9)\n");

    return -1;
  }

  if (n > TOP_HALFTONE) {
    fprintf(stderr, "n > 2 + 48\n");

    return -1;
  }

  n += -BOTTOM_HALFTONE;

  printf("\b\b\b");

  switch (n % 12) {
  case 0:  printf(" C"); break;
  case 1:  printf("C#"); break;
  case 2:  printf(" D"); break;
  case 3:  printf("D#"); break;
  case 4:  printf(" E"); break;
  case 5:  printf(" F"); break;
  case 6:  printf("F#"); break;
  case 7:  printf(" G"); break;
  case 8:  printf("G#"); break;
  case 9:  printf(" A"); break;
  case 10: printf("A#"); break;
  case 11: printf(" B");
  }

  printf("%d", n / 12);

  fflush(stdout);

  return 0;
}
