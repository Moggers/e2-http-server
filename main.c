#include "util/server.h"
#include "util/debug.h"
#include <string.h>

int main( int argc, char ** argv )
{
	server_start( argv[1] );
	getchar();
	return 0;
}
