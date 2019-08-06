/* bit manipulation macros */
#define BIT(x,b) (((x) & (1 << (b))) != 0)
#define SET(b) (1<<(b))
#define RESET(x,b) ((x) & ~(1 << (b)))

/* round a number 'n' up modulo 's' bytes */
#define ROUND(n,s) (((long)(n) + (s) - 1) & ~((s)-1))
