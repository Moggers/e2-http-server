#include "server.h"
#include <pthread.h>
#include "debug.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include "client_list.h"
#include "errno.h"

char * (*respond_callback) (char *);

void server_set_respond_callback( char * (*arg0)(char *) )
{
	respond_callback = arg0;
}

char * dummy_callback( char * arg0 )
{
	debug_printf( "HTTP callback method has not been set up!\n" );
	char * retmesg = calloc( 1024, sizeof( char ) );
	snprintf( retmesg, sizeof( char ) * 1024, "The HTTP response callback has not been set up yet, your request was: %s\n", arg0 );
	return retmesg;
}

typedef struct client_args
{
	int socketfd;
} client_args;

void * client_thread( void * arg0 )
{
	char * mesg;
	int err;
	mesg = calloc( 2048, sizeof( char ) );
	err = recv( ((client_args *)arg0)->socketfd, mesg, 2048, 0 );
	if( err == -1 )
	{
		debug_printf( "Failed to receive message from client %d: %s\n", ((client_args *)arg0)->socketfd, strerror( errno ) );
	}
	else if( err == 0 )
	{
		debug_printf( "Client on socket %d disconnected\n", ((client_args *)arg0)->socketfd );
		client_remove( ((client_args *)arg0)->socketfd );
		pthread_exit( NULL );
	}
	else
	{
		char * tmp = strtok( mesg, " " ); 
		char * callback_message = (strtok( NULL, " " ) + sizeof(char));
		char * respond_message = respond_callback( callback_message );
		send(((client_args *)arg0)->socketfd, "HTTP/1.1 200 OK\n\n", 17, 0);
		send(((client_args *)arg0)->socketfd, respond_message, strlen( respond_message ) * sizeof(char), 0);
		debug_printf( "Received message: %s\nResponded with message: %s\n", callback_message, respond_message );
		close( ((client_args *)arg0)->socketfd );
		client_remove( ((client_args *)arg0)->socketfd );
	}
}

typedef struct server_args
{
	char * port;
} server_args;

void * server_thread( void * arg0 )
{
	if( respond_callback == NULL )
	{
		respond_callback = dummy_callback;
	}
	debug_printf( "Server thread begun\n" );
    struct addrinfo hints, *res, *p;
	int listenfd;

    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
	int errcode;
	debug_printf( "Starting server on port %s\n", ((server_args *)arg0)->port );
    if ((errcode=getaddrinfo( NULL, ((server_args *)arg0)->port, &hints, &res)) != 0)
    {
        debug_printf("getaddrinfo() error:\n%s\n", gai_strerror(errcode));
		pthread_exit( NULL );
    }
    // socket and bind
    for (p = res; p!=NULL; p=p->ai_next)
    {
        listenfd = socket (p->ai_family, p->ai_socktype, 0);
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p==NULL)
    {
        debug_printf("socket() or bind()\n");
		pthread_exit( NULL );
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (listenfd, 1000000) != 0 )
    {
        debug_printf("listen() error\n");
		pthread_exit( NULL );
	}

	for( ;; )
	{
		struct sockaddr * cladd = calloc( 1, sizeof( struct sockaddr ) );
		socklen_t * cllength = calloc( 1, sizeof( socklen_t ) );
		int socketfd;
		if( (socketfd = accept( listenfd, cladd, cllength ) ) > 0 )
		{
			debug_printf( "Received connection\n" );
			client_add( socketfd );
			pthread_t * cl_thread = calloc( 1, sizeof( pthread_t ) );
			client_args args;
			args.socketfd = socketfd;
			pthread_create( cl_thread, NULL, client_thread, (void *)&args );
		}
		else
		{
			debug_printf( "Failure in accept():\n%s\n", strerror( errno ) );
		}
	}
	pthread_exit( NULL );
}

void server_start( char * port )
{
	pthread_t listen_thread;
	server_args * args;
	args = calloc( 1, sizeof( server_args ) );
	args->port = port;
	debug_printf( "Server begin\n" );
	pthread_create( &listen_thread, NULL, server_thread, (void*)args );
}

void server_destroy( void )
{
}
