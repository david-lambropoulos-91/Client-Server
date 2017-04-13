#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd>
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
		write( 1, message, sprintf( message, "\x1b[1;31msocket() failed. In file %s in function %s at line %d.\x1b[0m\n"), __FILE__, func, __LINE__ );
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
		write
	}
}
