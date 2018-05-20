#define MAXC 500

struct vertex {
  double x;
  double fx;
};

int cmpt(const struct vertex *p1, const struct vertex *p2);
double crest(double l, double x0, double x1, double fx0, double fx1);
double findl_fft(double a, double b, fftw_complex *out);
double piyavsky(double l, double e, double a, double b, double x0);
double vertex(double l, double a, double b, struct vertex *t, int k);
