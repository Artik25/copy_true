#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>

int find_substr( char substr[], char str[] ) {

	int i, j, substrLength, strLength;
	
	substrLength = strlen( substr );
	strLength = strlen( str );

	int *d = (int*)malloc( substrLength * sizeof( int ) );
	d[0] = 0;
	
	for ( i = 1, j = 0; i < substrLength; i++ ) {
		
		while ( j > 0 && substr[j] != substr[i] )
			j = d[ j - 1 ];

		if ( substr[j] == substr[i] ) j++;

		d[i] = j;
	}
	
	for ( i = 0, j = 0; i < strLength; i++ ) {

		while ( j > 0 && substr[j] != str[i] )
			j = d[ j - 1 ];
		
		if ( substr[j] == str[i] ) j++;

		if ( j == substrLength ) {

			free( d );
			return i - j + 1;
		}
	}
	
	free(d);
	return -1;
}
		
void grep ( char *substr, char *fileName ) {
	
	FILE * file = fopen( fileName, "r" );
	
	if ( file == NULL ) {
		fprintf( stderr, "Can't open file: %s\n", fileName );
		exit(1);
	}
	
	int BUFSIZE = 100;
	char *buf = malloc( BUFSIZE );
	
	if ( buf == NULL ) {
		fprintf( stderr, "Can't get memory" );
		exit(1);
	}
	
	int i = 0;

	while ( !feof( file ) ) {

		char c;
		c = fgetc( file );
		
		if ( c != '\n' ) {
			
			buf[ i++ ] = c;
			
			if ( i == BUFSIZE - 1 ) {
				buf = realloc( buf, 2 * BUFSIZE);
				BUFSIZE = 2 * BUFSIZE;
			}
		}

		else {
			buf[ i ] = '\0';

			if ( find_substr( substr, buf ) != -1 )
				printf ( " %s\n", buf );

			i = 0;
		}
	}
	
	free( buf );
	fclose( file );
}
	
void grepDir( char *substr, char* fileName ) {

	DIR *dir = opendir( fileName );
	char newPath[ PATH_MAX + 1 ];
	
	if ( dir == NULL ) {
		fprintf( stderr, "Can't open directory: %s\n", fileName );
		exit( 1 );
	}
		
	struct dirent* entry;

	while ( ( entry = readdir( dir ) ) != NULL ) {
		
		if ( entry->d_name[0] == '.' )
			continue;
		
		strcpy( newPath, fileName );
		strcat( newPath, "/" );
		strcat( newPath, entry->d_name );
	
		if ( entry->d_type & DT_DIR )
			grep( substr, newPath );
		if ( entry->d_type & DT_REG )
			grep( substr, newPath );
	}
	
	closedir( dir );
}			

int main(int argc, char **argv ) {
	
	if ( argc < 3 ) {
		fprintf( stderr, "Too few arguments" );
		exit(1);
	}

	int i;

	if ( strcmp( argv[1], "-R" ) == 0 ) 
		for ( i = 3; i < argc; i++ )
			grepDir( argv[2], argv[i] );
	
	else 
		for ( i = 2; i < argc; i++ )
			grep( argv[1], argv[i] );	
		
 	exit(0);
}
