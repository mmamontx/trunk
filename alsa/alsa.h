#define RATE            48000
#define BUFSIZE         385
#define PERIOD_SIZE     64
#define TOP_HALFTONE    2 + 48
#define BOTTOM_HALFTONE -(48 + 9)
#define N_HALFTONE      TOP_HALFTONE - BOTTOM_HALFTONE

short buf[PERIOD_SIZE * BUFSIZE * 2];
unsigned int mult;

double notes[N_HALFTONE];
const double pow_const = (double)1 / 12;
const double coeff_const = (double)2 * M_PI / RATE;

snd_pcm_uframes_t frames = PERIOD_SIZE;

char fnote(double f);
double goertzel(double f, short *buf, unsigned int len);
double note(double n);
int pnote(char n);
