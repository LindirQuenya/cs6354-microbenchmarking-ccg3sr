#ifndef MICROBENCH_UTIL_H
#define MICROBENCH_UTIL_H

// Well, this is kinda gross. But it'll do.
// Adapted from https://stackoverflow.com/a/8556436
#define COMMA ,
#define JOIN0(X)
#define JOIN1(X) X
#define JOIN2(X) JOIN1(X) COMMA X
#define JOIN3(X) JOIN2(X) COMMA X
#define JOIN4(X) JOIN3(X) COMMA X
#define JOIN5(X) JOIN4(X) COMMA X
#define JOIN6(X) JOIN5(X) COMMA X
#define JOIN7(X) JOIN6(X) COMMA X
#define JOIN8(X) JOIN7(X) COMMA X
#define JOIN9(X) JOIN8(X) COMMA X
#define JOIN10(X) JOIN9(X) COMMA X

#define JOIN(N,X) \
  JOIN##N(X)


#define REP0(X)
#define REP1(X) X
#define REP2(X) REP1(X) X
#define REP3(X) REP2(X) X
#define REP4(X) REP3(X) X
#define REP5(X) REP4(X) X
#define REP6(X) REP5(X) X
#define REP7(X) REP6(X) X
#define REP8(X) REP7(X) X
#define REP9(X) REP8(X) X
#define REP10(X) REP9(X) X

#define REP(HUNDREDS,TENS,ONES,X) \
  REP##HUNDREDS(REP10(REP10(X))) \
  REP##TENS(REP10(X)) \
  REP##ONES(X)

#endif