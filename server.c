#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <ctype.h>

#include "server.h"

static sem_t actionCycleSemaphore;
static pthread_mutex_t mutex;
static int connection_count = 0;
static int waiting_to_connect = 0;

int claim_port( const char* port )
{
	struct addrinfo addrinfo;
	struct addrinfo* result;
	int sd;
	char message[ RCVBUFSIZE ];
	int on = 1;
	char* func = "claim_port";

	addrinfo.ai_flags = AI_PASSIVE;	/* for bind() */
	addrinfo.ai_family = AF_INET;	/* IPv4 only */
	addrinfo.ai_socktype = SOCK_STREAM;	/* Want TCP/IP */
	addrinfo.ai_protocol = 0;	/* Any protocol */
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;

	if( getaddrinfo( 0, port, &addrinfo, &result ) != 0 )
	{
		fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed errno is %s. In file %s in function %s at line %d.\x1b[0m\n", port, strerror( errno ), __FILE__, func, __LINE__ );
		return -1;
	}
	else if( errno = 0, ( sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol ) ) == -1 )
	{
		write( 1, message, sprintf( message, "\x1b[1;31msocket() failed. In file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ ) );
		freeaddrinfo( result );
		return -1;
	}
	else if( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ) ) == -1 )
	{
		write( 1, message, sprintf( message, "\x1b[1;31msetsockopt() failed. In file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ ) );
		freeaddrinfo( result );
		close( sd );
		return -1;
	}
	else if( bind( sd, result->ai_addr, result->ai_addrlen ) == -1 )
	{
		freeaddrinfo( result );
		close( sd );
		write( 1, message, sprintf( message, "\x1b[2;33mBinding to port %s ...\x1b[0m\n", port ) );
		return -1;
	}
	else
	{
		write( 1, message, sprintf( message, "\x1b[1;32mSUCCESS : Bind to port %s\x1b[0m\n", port ) );
		freeaddrinfo( result );
		return sd;	// bind() succeeded
	}
}


static char *ps( unsigned int x, char* s, char* p )
{
	return x == 1 ? s : p;
}

void periodic_action_handler( int signo, siginfo_t* ignore, void* ignore2 )
{
	if( signo == SIGALRM )
	{
		sem_post( &actionCycleSemaphore );
	}
}

void* periodic_action_cycle_thread( void* ignore )
{

	return 0;
}

void* client_session_thread( void* args )
{
	int sd;
	char request[ RCVBUFSIZE ];
	char response[ RCVBUFSIZE ];

	return 0;
}

int main(int argc, char** argv)
{
	int sd;
	char message[ RCVBUFSIZE ];
	char temp[ RCVBUFSIZE ];
	pthread_t tid;
	pthread_attr_t kernel_attr;
	socklen_t ic;
	int fd;
	struct sockaddr_in senderAddr;
	int*	fdptr;
	char* func = "main";

	if(argc < 2)
	{
		sprintf( temp, "\x1b[1;31mNo port provided!!!!\x1b[0m\n" );
		ERR_EXIT( temp );
	}

	if( pthread_attr_init( &kernel_attr ) != 0 )
	{
		sprintf( temp, "\x1b[1;31mpthread_attr_init() failed in file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		ERR_EXIT( temp );
	}
	else if( pthread_attr_setscope( &kernel_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
	{
		sprintf( temp, "\x1b[1;31mpthread_attr_setscope() failed in file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		ERR_EXIT( temp );
	}
	else if( ( sd = claim_port( argv[ 1 ] ) ) == -1 )
	{
		sprintf( temp, "\x1b[1;31mCould not bind to port %s errno %s in file %s in function %s at line %d.\x1b[0m\n", argv[ 1 ], strerror( errno ), __FILE__, func, __LINE__ );
		ERR_EXIT( temp );
	}
	else if( listen( sd, 100 ) == -1 )
	{
		sprintf( temp, "\x1b[1;31mlisten() failed in file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		close( sd );
		ERR_EXIT( temp );
	}
	else
	{
		ic = sizeof( senderAddr );

		while( ( fd = accept( sd, ( struct sockaddr * ) &senderAddr, &ic ) ) != -1 )
		{
			fdptr = ( int* ) malloc( sizeof( int ) );
			*fdptr = fd;

			if( pthread_create( &tid, &kernel_attr, client_session_thread, fdptr ) != 0 )
			{
				sprintf( temp, "\x1b[1;31mpthread_create() failed in file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
				ERR_EXIT( temp );
			}
			else if( pthread_create( &tid, &kernel_attr, periodic_action_cycle_thread, 0 ) != 0 )
			{
				sprintf( temp, "\x1b[1;31mpthread_create() failed in file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
				ERR_EXIT( temp );
			}
			else
			{
				continue;
			}
		}
		close( sd );
		return 0;
	}
	


	return 0;
}
