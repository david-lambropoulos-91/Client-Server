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
	struct sigaction action;
	struct itimerval interval;

	pthread_detach( pthread_self( ) );
	action.sa_flags = SA_SIGINFO | SA_RESTART;
	action.sa_sigaction = periodic_action_handler;
	sigemptyset( &action.sa_mask );
	sigaction( SIGALRM, &action, 0 );	/* invoke periodic_action_handler() when timer expires */
	interval.it_interval.tv_sec = 20;
	interval.it_interval.tv_usec = 0;
	interval.it_value.tv_sec = 20;
	interval.it_value.tv_usec = 0;

	setitimer( ITIMER_REAL, &interval, 0 );	/* every 3 seconds */

	for(;;)
	{
		sem_wait( &actionCycleSemaphore );
		pthread_mutex_lock( &mutex );
		printf( "There %s %d active %s.\n", ps( connection_count, "is", "are" ),
			connection_count, ps( connection_count, "connection", "connections" ) );
		pthread_mutex_unlock( &mutex );
		sched_yield();
	}

	return 0;
}

void* client_session_thread( void* arg )
{
	int sd;
	char request[ RCVBUFSIZE ];
	char response[ RCVBUFSIZE ];
	char* dummy;
	char* temp;
	sd = *(int*) arg;
	free(arg);
	pthread_detach( pthread_self() );		// Don't join on this thread

//	if(connection_count == 0)
//	{
		pthread_mutex_lock( &mutex );
		++connection_count;				// multiple clients protected access
		pthread_mutex_unlock( &mutex );
//	}
//	else
//	{
//		pthread_mutex_lock( &mutex );
///		++waiting_to_connect;				// multiple clients protected access
//		pthread_mutex_unlock( &mutex );
//	}

	while ( read( sd, request, sizeof(request) ) > 0 )
	{
		printf( "server receives input:  %s\n", request );
		
		if(dummy = strtok(request," "), dummy == NULL)
		{
			strcpy(response, "NOTHING ENTERED");
		}
		else if(strcmp(dummy, "BOUNCE") == 0)
		{
			if(dummy = strtok(NULL," "), dummy == NULL)
			{
				strcpy(response, "NO ARGUMENTS PROVIDED TO BOUNCE");
			}
			else
			{
				while(temp = strtok(NULL, "\0"), temp != NULL)
				{
					sprintf(dummy, "%s %s", dummy, temp++);
				}
				strcpy(response, dummy);
				printf("\n%s\n", dummy);
			}
		}
		else if(strcmp(dummy, "GET") == 0)
		{
			if(dummy = strtok(NULL," "), dummy == NULL)
			{
				strcpy(response, "NO ARGUMENTS PROVIDED TO GET");
			}
			else
			{
				printf("\n%s\n", dummy);
				FILE *fp;
				fp = fopen(dummy,"r");
				size_t nread;

				if(fp != NULL)
				{
					while(( nread = fread( response, 1, sizeof(response), fp ) ) > 0)
					{
						write(sd, response, strlen(response)+1);
					}
					if(ferror(fp))
					{
				
					}	
				}
				else
				{
					strcpy(response, "FILE DOES NOT EXIST");
				}
			}
				
		}
		else if(strcmp(dummy, "EXIT") == 0)
		{
			if( dummy = strtok( NULL, "\0" ), dummy == NULL )
			{
				strcpy(response, "EXIT");
			}
			else
			{
				while( temp = strtok( NULL, "\0" ), temp != NULL )
				{
					sprintf(dummy, "%s %s", dummy, temp++);
				}
				
				//strcpy(response, dummy);		
				printf("Exit message: %s\n", dummy);
				//write( sd, response, strlen(response)+1);
				sprintf(response, "EXIT");
			}
		}
		else
		{
			sprintf(response, "Not a valid command!!!\n");
		}

		write(sd, response, strlen(response)+1);
	}

	pthread_mutex_lock( &mutex );
	--connection_count;				// multiple clients protected access
	pthread_mutex_unlock( &mutex );

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
		
		while(connection_count != 0)	
		{
			sleep(2);
		}

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
}
