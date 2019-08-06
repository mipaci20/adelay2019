#ifndef _util_h
#define _util_h
#include "types.h"
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#define MAX_32bit 4294967296.

extern FILE    *FILE_open(char *name, char *ext, char *mode);
extern int     bit_set(int old, int pos, int bit);
extern void    dmsg(char *fmt, ...);
extern char *  expand_tilde(char *fn, char *buffer);
extern long    file_size(char *path);
extern char *  ftoa(double);
extern void    gettimeofday_ntp(u_int32 *);
extern double  gettimeofday_double(void);
extern char *  getusername(void);
extern char *  itoa(int);
extern char *  inet_Ntoa(u_int32);
extern int     idletime(void);
extern int     msb_index(int);
extern int     open_file(char *dir, char *name, char *ext, char *mode, char **chosen);
extern char *  p2t(double t, int decimals);
extern u_int32 random32(int);
extern double  rtcp_period(int members, int senders, double bw, int
               we_sent, int packet_size, int *avg_control, int initial);
extern char    *skip_space(char *s);
extern int     split_line(char *line, char *sep, char *argv[], int count);
extern char *  strappend(char **, char *);
extern char *  strappendn(char **, char *, int);
extern int     strcpy_pad(char *, char *);
extern void    string_DES_key(char *key, u_int8 des_key[8], char algorithm[16]);
extern char *  strsave(const char *);
extern char *  strsaven(const char *, int len);
extern char *  strtox(char *);
extern u_int32 timeval_ntp32(struct timeval *tv);

#ifdef _TCL
extern int source_file(Tcl_Interp *interp, char *name);
extern void tcl(Tcl_Interp *interp, char *fmt, ...);
#endif

/*
* Return number of bytes needed to pad to next N-byte boundary (0..N-1).
*/
#define PAD(x,n) (((n) - ((x) & (n-1))) & (n-1))

#endif
