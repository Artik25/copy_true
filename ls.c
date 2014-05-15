#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

void changeMode( int mode, char str[])
{
	strcpy( str, "--------" );
	if ( S_ISDIR(mode)) str[0] = 'd';
	if ( S_ISCHR(mode)) str[0] = 'c';
	if ( S_ISBLK(mode))  str[0] = 'b';
	if ( mode & S_IRUSR) str[1] = 'r';
	if ( mode & S_IWUSR) str[2] = 'w';
	if ( mode & S_IXUSR) str[3] = 'x';
	if ( mode & S_IRGRP) str[4] = 'r';
	if ( mode & S_IWGRP) str[5] = 'w';
	if ( mode & S_IXGRP) str[6] = 'x';
	if ( mode & S_IROTH) str[7] = 'r';
	if ( mode & S_IWOTH) str[8] = 'w';
	if ( mode & S_IXOTH) str[9] = 'x';
}

char *uidToName( uid_t uid )
{
	struct passwd *getpwuid(), *pw_ptr;
	static char numstr[10];
	if (( pw_ptr = getpwuid( uid )) == NULL)
	{
		sprintf( numstr,"% d", uid );
		return numstr;
	}
	else
		return pw_ptr->pw_name;
}

char *gidToName( gid_t gid )
{
	struct group *getgrgid(), *grp_ptr;
	static char numstr[10];
	if ((grp_ptr = getgrgid( gid )) == NULL)
	{
		sprintf(numstr,"% d", gid);
		return numstr;
	}
	else
		return grp_ptr->gr_name;
}

void lookDir( char *, int );

void lookGlobal( char *path, int allInform ) 
{
	DIR *dir = opendir( path );
	struct stat inf;
	struct dirent *entry;
	char newPath[ PATH_MAX + 1 ];
	
	printf( "\n\033[36;1m%s\033[0m:\n", path );
	lookDir( path, allInform );
	
	while( ( entry = readdir( dir )) != NULL ) 
	{ 	
		if ( entry->d_name[0] == '.' ) continue;

		strncpy( newPath, path, PATH_MAX );
		strncat( newPath, entry->d_name, PATH_MAX );
		
		if ( !strcmp( path, "." ) )
			strncpy( newPath, entry->d_name, PATH_MAX ); 		
 
		if( stat( newPath, &inf ) < 0 )
		{
			fprintf( stderr, "Cannot read file '%s'\n", newPath );
			exit(1);
		}
			
		if ( inf.st_mode & S_IFDIR )
		{
			strncat( newPath, "/", PATH_MAX );
			lookGlobal( newPath, allInform );
		}
	}
}

void lookDir( char *path, int allInform )
{
	DIR *dir;
	struct stat inf;	
	struct dirent *entry;
	char modestr[11];
		
	dir = opendir( path );
	
	if ( !dir )
 	{
     		fprintf ( stderr, "Cannot open file '%s'\n", path );
       	        exit(1);	
        }

        while ( ( entry = readdir( dir )) != NULL )
     	{	
		if ( entry->d_name[0] == '.' ) continue;

                char newPath[ PATH_MAX + 1 ];
       	 
		strncpy( newPath, path, PATH_MAX );
          	strncat( newPath, "/", PATH_MAX );
      		strncat( newPath, entry->d_name, PATH_MAX );

                if ( stat( newPath, &inf ) < 0 )
                {
	               	 fprintf ( stderr, "ls: Cannot open file\n");
              	         exit(1);
        	}
		
		if ( allInform )
		{	
			changeMode( inf.st_mode, modestr );
              
               		printf( "%s %4d %-8s %-8s %8ld %.12s \033[34m%s\033[0m\n", modestr, (int)inf.st_nlink, uidToName( inf.st_uid ), 
			gidToName( inf.st_gid ), (long)inf.st_size, ctime( &inf.st_ctime), entry->d_name );
                }
		else
		{ 	
			if ( inf.st_mode & S_IFDIR )
				printf( "\033[34m%s\033[0m\n", entry->d_name );
			if ( inf.st_mode & S_IFREG )
				printf( "%s\n", entry->d_name );
		}
	}
}	                    	 
		
int main( int argc, char ** argv ) 
{	
	int i, idOfPath;
	int isItGlobal = 0, allInform = 0, isItPath = 0; 
	struct stat inf;
	
	for ( i = 1; i < argc; i++ )
	{	
		if ( !strcmp( argv[i], "-R" ) ) 
		{
			isItGlobal = 1;
			continue;
		}

		if ( !strcmp( argv[i], "-l" ) )
		{
			allInform = 1;
			continue;
		}
		
		isItPath = 1;
		idOfPath = i;
	}
		
	if ( argc == 1 )
	{	
		lookDir( ".", allInform );
		exit(0);
	}
	
	if ( isItPath )
	{
		
		if ( stat( argv[ idOfPath ], &inf ) < 0 )
		{
			fprintf( stderr, "Cannot open file: '%s'\n", argv[ idOfPath ] );
			exit(1);
		}
		
		if ( inf.st_mode & S_IFDIR )
		{ 	

			if ( isItGlobal )
				lookGlobal( argv[ idOfPath ], allInform );
			else
				lookDir( argv[ idOfPath ], allInform );
		}

		else
		{
			if ( allInform )
				printf( "FILE   %8s %ld %s\n", argv[ idOfPath ], ( long ) inf.st_size, ctime( &inf.st_ctime) );
			else
				printf( "%s\n", argv[ idOfPath ] );
		} 
		
	}
	
	else 
	{
		if ( isItGlobal )	
			lookGlobal( ".", allInform );
		else
			lookDir( ".", allInform );
	}	

	exit(0);
}
