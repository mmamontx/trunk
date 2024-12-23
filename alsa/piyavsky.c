/* April-05-14. Mikhail Mamontov. */

#include "piyavsky.h"

int cmpt(const struct vertex *p1, const struct vertex *p2)
{
  if (p1->x == p2->x)
    return 0;

  return p1->x > p2->x ? 1 : -1;
}

double crest(double l, double x0, double x1, double fx0, double fx1)
{
  double x;

  if (x1 > x0)
    x = fx1 - fx0 + (x0 + x1) * l;
  else
    x = fx0 - fx1 + (x0 + x1) * l;

  x /= 2 * l;

  return x;
}

double findl_fft(double a, double b, fftw_complex *out)
{
  int x, xm;
  double l, lm;

  for (x = a + 1; x < b; x++) {
    l = fabs(*out[x] - *out[x - 1]);

    if (x == a + 1) {
      lm = l;
      xm = x;

      continue;
    }

    if (l > lm) {
      lm = l;
      xm = x;
    }
  }

  return lm;
}

double piyavsky(double l, double e, double a, double b, double x0)
{
  int k;
  double x;
  const double count = log2(PERIOD_SIZE * BUFSIZE * 2) * 5 / 6;
  struct vertex fstar;
  struct vertex *t;

  /*printf("l = %f e = %f a = %f b = %f x0 = %f\n", l, e, a, b, x0);*/

  if ((t = (struct vertex *)malloc(sizeof(struct vertex) * (int)count)) == NULL) {
    fprintf(stderr, "malloc() returned NULL\n");

    exit(-1);
  }

  fstar.x = t[0].x = x0;
  fstar.fx = t[0].fx = 10 * log10(goertzel(x0, buf, frames * mult));

#ifndef GRAPH
  /*printf("  0) fx(%4.0f) = %f\n", t[0].x, t[0].fx);*/
#endif
#ifdef GRAPH
  printf("%f + |x - %f|;\n", t[0].fx, t[0].x);
#endif

  /*for (k = 1; k < MAXC; k++) {*/
  for (k = 1; k < (int)count; k++) {
    if (k > 1)
      qsort(t, k, sizeof(struct vertex), (__compar_fn_t)cmpt);

    x = vertex(l, a, b, t, k);

    t[k].x = x;
    t[k].fx = 10 * log10(goertzel(x, buf, frames * mult));

#ifndef GRAPH
    /*printf("%3d) fx(%4.0f) = %f\n", k, t[k].x, t[k].fx);*/
#endif
#ifdef GRAPH
    printf("%f + |x - %f|;\n", t[k].fx, t[k].x);
#endif

    if (fabs(t[k].x - fstar.x) < e) {
      /*printf("|x(k) - x*(k)| < %f\n", e);*/

      return t[k].x;
    }

    if (t[k].fx > fstar.fx) {
#ifndef GRAPH
      /*puts("***********************");
      printf("%f dB > %f dB; fstar.x := %f\n", t[k].fx, fstar.fx, t[k].x);
      puts("***********************");*/
#endif
      fstar = t[k];
    }
  }

  free(t);

  return fstar.x;
}

struct vertex px(double l, double a, double b, double x0, double fx0)
{
  int i;
  double v[3] = {a, x0, b};
  struct vertex x, xm;

  for (i = 0; i < 3; i++) {
    x.fx = fabs(v[i] - x0);
    x.fx = fx0 + l * x.fx;

    if (i == 0) {
      xm.x = v[i];
      xm.fx = x.fx;

      continue;
    }

    if (x.fx > xm.fx) {
      xm.x = v[i];
      xm.fx = x.fx;
    }
  }

  return xm;
}

double vertex(double l, double a, double b, struct vertex *t, int k)
{
  int i;
  double c, cp;
  struct vertex x, xm;

  for (i = 0; i < k; i++, cp = c) {
    if ((k > 1) && (i < k - 1))
      c = crest(l, t[i].x, t[i + 1].x, t[i].fx, t[i + 1].fx);

    if (i == 0) {
      xm = px(l, a, k > 1 ? c : b, t[i].x, t[i].fx);

      continue;
    }

    x = px(l, cp, i != k - 1 ? c : b, t[i].x, t[i].fx);

    if (x.fx > xm.fx)
      xm = x;
  }

  return xm.x;
}
