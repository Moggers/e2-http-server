#include "client_list.h"
#include <stdlib.h>
#include <string.h>

typedef struct client_struct
{
	int socketfd;
	struct client_struct * n;
} client_struct;

client_struct * client_ll;

void client_add( int socketfd )
{
	if( client_ll == NULL )
	{
		client_ll = calloc( 1, sizeof( client_struct ) );
		client_ll->socketfd = socketfd;
		return;
	}
	client_struct * new = calloc( 1, sizeof( client_struct ) );
	new->socketfd = socketfd;
	client_struct * cur = client_ll;
	while( cur->n != NULL )
	{
		cur = cur->n;
	}
	cur->n = new;
}

void client_remove( int socketfd )
{
	client_struct * cur = client_ll;
	client_struct * p;
	while( cur->socketfd != socketfd && cur->n != NULL )
	{
		p = cur;
		cur = cur->n;
	}
	if( cur->socketfd == socketfd )
	{
		if( client_ll == cur )
		{
			client_ll = cur->n;
		}
		else
		{
			p->n = cur->n;
			free( cur );
		}
	}
}
