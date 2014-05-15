#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define BUFFER_SIZE     64

void copyFile( char *path1, char *path2 )
{	
	ssize_t read_bytes;
	struct stat inform;
	char buffer[BUFFER_SIZE + 1];

	if ( stat( path1, &inform ) == -1 )
	{
		fprintf( stderr, "Cannot open file: '%s'\n", path1 );	
		return;
	}
	
	int fd1 = open( path1, O_RDONLY );
	int fd2 = open( path2, O_WRONLY | O_CREAT, inform.st_mode );

	if ( fd1 < 0 || fd2 < 0 ) 
	{
		fprintf( stderr, "Cannot open file\n");
		return;
	}

	while (( read_bytes = read( fd1, &buffer, BUFFER_SIZE )) > 0 )
	{
		if ( write( fd2, &buffer, read_bytes) < 0 )
		{
			printf ( "Cannot write\n");
			return;
		}
	}
	
	close(fd1);
	close(fd2);
}	

void copyDir( char *path1, char *path2 ) 
{
	 DIR *dir1, *dir2;
         struct dirent *entry;
         struct stat inform;
         char newPath1[ PATH_MAX + 1];
         char newPath2[ PATH_MAX + 1];
	               
	 dir1 = opendir( path1 );
       
	 if ( !dir1 )
         {
         	fprintf ( stderr, "Cannot open file '%s'\n", path1 );
      	        exit(1);	
       	 }

	 if ( stat( path1, &inform ) == -1 )
         {
		fprintf( stderr, "Cannot open directory '%s'\n", path1 );
                exit(1);
         }
       
	 if ( mkdir( path2, inform.st_mode ) < 0 )
         {
        	fprintf( stderr, "Cannot make directory '%s'\n", path2 );
                exit(1);
         }

         while ( ( entry = readdir( dir1 ) ) != NULL )
         {	
         	if ( entry->d_name[0] == '.')
   	                continue;
        	strncpy( newPath1, path1, PATH_MAX );
               	strncpy( newPath2, path2, PATH_MAX);
               	strncat( newPath1, "/", PATH_MAX );
           	strncat( newPath2, "/", PATH_MAX );
                strncat( newPath1, entry->d_name, PATH_MAX );
      	        strncat( newPath2, entry->d_name, PATH_MAX );

                if ( stat( newPath1, &inform ) < 0 )
                {
	            	 fprintf ( stderr, "Cannot open file\n");
                         continue;
        	}

                if (inform.st_mode & S_IFREG)
                      	 copyFile( newPath1, newPath2 );
	
		if (inform.st_mode & S_IFDIR)
			copyDir( newPath1, newPath2 );		
     	}

        closedir( dir1 );
}


int main( int argc, char **argv ) {

	struct stat inform;
	char path[ PATH_MAX + 1 ];	

	if ( argc < 3 ) 
	{
		fprintf (stderr, "Too few arguments\n");
		exit (1);
	}
	
	if ( stat( argv[1] , &inform ) < 0 )
        {
             	fprintf ( stderr, " Cannot open file '%s'\n", argv[1] );
     	        exit(1);
        }
	
	if ( inform.st_mode & S_IFREG )
		copyFile( argv[1], argv[2] );
	else 
	{
		if ( opendir( argv[2] ) ) 
		{
			strncpy( path, argv[2], PATH_MAX );
			strncat( path, "/", PATH_MAX );
			strncat( path, argv[1], PATH_MAX );
			
			copyDir( argv[1], path );
		}	
		else
			copyDir( argv[1], argv[2] );
	}		
	exit(0);
}
	
		
	
	
