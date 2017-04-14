#define RCVBUFSIZE 256 /* Size of receive buffer */
#define ERR_EXIT(msg) { perror(msg); exit(1); }

/**
*	Given the name of a server and a port that it is to be connected to.
*	Establish a connection and return a value corresponding to the success.
*	-1 = failure, 0 = success.
**/
int connect_to_server(const char* server, const char* port);


/**
*	A thread that handles input from the user and delivers the input to 
*	the server.
**/
void* command_input_thread(void* arg);

/**
*	A thread that handles output from the server and presents it to the
*	user.
**/
void* response_output_thread(void* arg);
