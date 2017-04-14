#include <sys/types.h>
#include <stdio.h>	/* for perror(), fprintf(), sprintf() */
#include <stdlib.h>	/* for atoi() */
#include <unistd.h>	/* for close() */
#include <errno.h>
#include <string.h>	/* for memset(), memcpy(), strlen() */
#include <sys/socket.h> /* for sockaddr, socket(), connect(), recv(),
			   send(), htonl(), htons() */
#include <netdb.h>	/* for hostent, gethostbyname() */
#include <pthread.h>
#include <time.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <signal.h>
#include <arpa/inet.h> /* for sockaddr_in */

#include "client.h"

static int sd;

int connect_to_server( const char* server, const char* port )
{
	int sd;	//socket device
	struct addrinfo addrinfo;
	struct addrinfo* result;
	char message[ RCVBUFSIZE ];
	char* func = "connect_to_server";

	// Configure the addrinfo structure
	addrinfo.ai_flags = 0;
	addrinfo.ai_family = AF_INET;	// IPv4 only
	addrinfo.ai_socktype = SOCK_STREAM;	// want TCP/IP
	addrinfo.ai_protocol = 0;
	addrinfo.ai_addrlen = 0;
	addrinfo.ai_addr = NULL;
	addrinfo.ai_canonname = NULL;
	addrinfo.ai_next = NULL;

	if( getaddrinfo( server, port, &addrinfo, &result ) != 0 )
	{
		fprintf( stderr, "\x1b[1;31mgetaddrinfo( %s ) failed.  In file %s, in function %s at line %d.\x1b[0m\n", server, __FILE__, func, __LINE__ );
		return -1;
	}
	else if( errno = 0, ( sd = socket( result->ai_family, result->ai_socktype, result->ai_protocol ) ) == -1 )
	{
		freeaddrinfo( result );
		return -1;
	}
	else
	{
		do
		{
			if ( errno = 0, connect( sd, result->ai_addr, result->ai_addrlen ) == -1 )
			{
				sleep( 3 );
				write( 1, message, sprintf( message, "\x1b[2;33mConnecting to server %s ...\x1b[0m\n", server ) );
			}
			else
			{
				freeaddrinfo( result );
				return sd;	// connect() succeeded
			}
		} while ( errno == ECONNREFUSED );
	
		freeaddrinfo( result );
		result -1;
	}
}

void* command_input_thread( void* arg )
{
	char* func = "command_input_thread";
	char string[ RCVBUFSIZE ];
	char prompt[ ] = "";
	int len;

	pthread_detach( pthread_self( ) );

	while( write( 1, prompt, sizeof( prompt ) ), ( len = read( 0, string, sizeof( string ) ) ) > 0 )
	{
		string[ len - 1 ] = '\0';
		write( sd, string, strlen( string ) + 1 );

		if( strcmp( string, "EXIT" ) == 0)
		{
			
		}

		sleep( 2 );	
	}

	return 0;
}

void* response_output_thread( void* arg )
{
	char* func = "response_output_thread";
	char output[ RCVBUFSIZE ];
	char buffer[ RCVBUFSIZE ];
	char temp[ RCVBUFSIZE ];
	int len;

	pthread_detach( pthread_self(  ) );

	while( len = read( sd, buffer, sizeof( buffer ) ), len >= 0 )
	{
		if( strcmp( buffer, "EXIT" ) == 0 )
		{
			strcpy( output, "\nGoodbye! :)\n\n" );
			write( 1, output, strlen( output ) );
			return 0;
		}
		
		if( len == 0 )
		{
			sprintf( temp, "\x1b[1;31m\n\nServer has unexpectedly crashed! In file %s in function %s at line %d \n\n", __FILE__, func, __LINE__ );
			ERR_EXIT( temp );
		}

		sprintf( output, "\nResult is >%s<\n", buffer );
		write( 1, output, strlen( output ) );
	}

	return 0;
}

int main( int argc, char** argv )
{
	char message[ RCVBUFSIZE ];
	char temp[ RCVBUFSIZE ];
	char* func = "main";
	pthread_t tid;
	pthread_attr_t kernel_attr;
	
	if( argc < 2 )
	{
		sprintf( temp, "\x1b[1;31mNo host name specified. In file %s, in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		ERR_EXIT( temp );
	}
	else if( argc < 3 )
	{
		sprintf( temp, "\x1b[1;31mNo port number specified. In file %s, in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		ERR_EXIT( temp );
	}
	
	if( pthread_attr_init( &kernel_attr ) != 0 )
	{
		sprintf( temp, "\x1b[1;31mpthread_attr_init() failed in file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		ERR_EXIT( temp );	
	}
	else if( pthread_attr_setscope( &kernel_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
	{
		sprintf( temp, "\x1b[1;31pthread_attr_setscope() failed in file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		ERR_EXIT( temp );
	}
	else if( ( sd = connect_to_server( argv[ 1 ], argv[ 2 ] ) ) == -1 )
	{
		sprintf( temp, "\x1b[1;31mCould not establish a connect to %s on port %s, in file %s, in function %s at line %d.\x1b[0m\n", argv[ 1 ], argv[ 2 ], __FILE__, func, __LINE__ );
		ERR_EXIT( temp );
	}
	else
	{
		printf( "Connected to server %s on port %s", argv[ 1 ], argv[ 2 ] );
		if (pthread_create(&tid, &kernel_attr, command_input_thread, 0) != 0 )
		{
			sprintf( temp, "\x1b[1;31mpthread_create() failed in file %s line %d\x1b[0m\n", __FILE__, __LINE__ );
			ERR_EXIT( temp );
		}
		else if(pthread_create( &tid, &kernel_attr, response_output_thread, 0) != 0 )
		{
			sprintf( temp, "\x1b[1;31mpthread_create() failed in %s()\x1b[0m\n", func );
			ERR_EXIT( temp );
		}
		pthread_exit( 0 );
		close( sd );
		return 0;	
	}

	return 0;
}
