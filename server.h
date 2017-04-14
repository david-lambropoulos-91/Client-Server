#define MAXPENDING 5	/* Max outstanding connection requests */
#define RCVBUFSIZE 256	/* Size of receive buffer */
#define ERR_EXIT(msg) { perror(msg); exit(1); }
