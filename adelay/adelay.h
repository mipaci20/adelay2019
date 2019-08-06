/* adelay.h */

/*
 * Calculate delay from the specified time and the length
 */
static int search_spurt(int *start, int *spurt, int *silence);

/*
 * Calculate delay from the specified time and the length
 * The unit of start and length is [msec]
 */
int cal_delay(int num, int start, int length);

/*
 * Handles interrupt signal: close the opened audio device
 */
static void sigint(int sig);

/*
 * Given frequency and time length
 * Return the data length as power of 2 just bigger than freq * time
 */
static int getDataLength(int freq, float time);

/*
 * Print the lag to out
 */
static void printLag(FILE * out, int lag, int freq, float time);

/*
 * Print results to out in "lag	Correlation"
 * If printData is true, in "lag Correlation LeftChannel RightChannel"
 */
static void printResult(FILE * out, float * answer, float * data1, 
			float * data2, int len, int freq, int printData);

/* 
 * Parse command line string
 */
static int parseCmd(int argc, char *argv[]);

/*
 * Print usage and quit
 */
static void usage(const char * pname);

