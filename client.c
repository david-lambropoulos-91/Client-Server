#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <signal.h>

int connect_to_server(const char* server, const char* port)
{
	int sd;	//socket device
	struct addrinfo addrinfo;
	struct addrinfo* result;
	char message[256];
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

int main(int argc, char** argv)
{
	char message[256];
	char* func = "main";
	int sd;
	pthread_t tid;
	pthread_attr_t kernel_attr;
	
	if( argc < 2 )
	{
		fprintf( stderr, "\x1b[1;31mNo host name specified. In file %s, in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__);
		exit( 1 );
	}
	else if( argc < 3 )
	{
		fprintf( stderr, "\x1b[1;31mNo port number specified. In file %s, in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		exit( 1 );
	}
	
	if( pthread_attr_init( &kernel_attr ) != 0 )
	{
		fprintf( stderr, "\x1b[1;31mpthread_attr_init() failed in file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		exit( 1 );	
	}
	else if( pthread_attr_setscope( &kernel_attr, PTHREAD_SCOPE_SYSTEM ) != 0 )
	{
		fprintf( stderr, "\x1b[1;31pthread_attr_setscope() failed in file %s in function %s at line %d.\x1b[0m\n", __FILE__, func, __LINE__ );
		exit( 1 );
	}
	else if( ( sd = connect_to_server( argv[1], argv[2] ) ) == -1 )
	{
		fprintf( stderr, "\x1b[1;31mCould not establish a connect to %s on port %s, in file %s, in function %s at line %d.\x1b[0m\n", argv[1], argv[2], __FILE__, func, __LINE__ );
		exit( 1 );
	}
	else
	{
		printf( "Connected to server %s on port %s", argv[1], argv[2] );	
	}

	return 0;
}
