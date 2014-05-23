#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/timeb.h>
#include <signal.h>
#include <sys/wait.h>

struct symbols{
    int pos;
    char symbol;
};

void setup() {
	
	signal( SIGINT, SIG_IGN );
	signal( SIGQUIT, SIG_IGN );
}

void fatal( char *s1, char *s2, int n ) {
	
	fprintf( stderr, " Error: %s, %s\n", s1, s2 );
	exit(n);
}

//освобождем массив аргументов
void freeList( char **list ) {
	
	char **cp = list;
	
	while (*cp )
		free( *cp++ );
	
	free( list );
}

void *emalloc ( size_t n ) {
    
	void *rv;
	if ( (rv = malloc( n )) == NULL )
		fatal( "out of memory", "", 1 );
	return rv;
}

void *erealloc ( void *p, size_t n ) {
    
	void *rv;
	if (( rv = realloc( p, n )) == NULL )
		fatal( "realloc() failed", "", 1 );
	return rv;
}

//выбирает подстроку из строки
char *newStr( char *s, int len ) {
    
	char *rv = emalloc( len + 1 );
	rv[ len ] = '\0';
	strncpy( rv, s, len );
	return rv;
}
// читаем следующую комндную строку из fp
char *nextComand( FILE *fp ) {
	
	char *buf;
	int bufspace = 0;
	int pos = 0;
	int c;
	
	printf ( "%c", '$' );
	
	while ( ( c = getc( fp ) ) != EOF ) {
        
		if ( pos + 1 >= bufspace ) {
            
			if ( bufspace == 0 )
				buf = emalloc( BUFSIZ );
			else
				buf = realloc( buf, bufspace + BUFSIZ );
            
			bufspace += BUFSIZ;
		}
		
		if ( c == '\n' )
			break;
		
		buf[ pos++ ] = c;
	}
	
	if ( c == EOF && pos == 0 )
		return NULL;
	
	buf[ pos ] = '\0';
	return buf;
}

//разделяет строку на команды и возвращает массив:
#define is_delim(x) ( (x) == ' ' || (x) == '\t' )
char **splitLine( char *line, struct symbols *sym ) {
   
	char **args;
	int spots = 0;  // кол-во свободных мест в массиве
	int bufspace = 0;
	int argnum = 0;
	char *cp = line;
	char *start;
	int len;
    
	if ( line == NULL )
		return NULL;
	
	args = emalloc( BUFSIZ );
	bufspace = BUFSIZ;
	spots = BUFSIZ / sizeof( char *);
	
	while ( *cp != '\0' ) {
        
		while( is_delim( *cp )) cp++; //пропускаем начальные пробелы
		
		if ( *cp == '\0' )
			break;
		
		if ( argnum + 1 > spots ) {
            
			args = erealloc( args, bufspace + BUFSIZ );
			bufspace += BUFSIZ;
			spots += ( BUFSIZ / sizeof ( char *));
		}
		
		start = cp;
		len = 1;
		
		while ( *++cp != '\0' && !( is_delim( *cp ) ) )
			len++;
        	
		if ( len == 1 ) {
            	 	
			cp--;			
			if( *cp == '>' ) {
                		sym->pos = argnum;
                		sym->symbol = '>';
                
            		}
            		
			else if( *cp == '<' ) {
                		sym->pos = argnum;
                		sym->symbol = '<';
            		}
                	else if( *cp == '|' ) {
                    		sym->pos = argnum;
                    		sym->symbol = '|';
                	}
			cp++;
		}
        
		args[ argnum++ ] = newStr( start, len );
		
	}
	args[ argnum ] = NULL;
	return args;
}

//Запуск программ
int execute( char *argv[] , char *comand) {
    
	int pid;
    
	struct symbols sym;
   	sym.pos = -1;
    
    	if ( ( argv = splitLine( comand, &sym ) ) == NULL )
        	return 0;
    
	int childInfo = -1;
    
	if( argv[0] == NULL )
		return 0;
    
	if ((pid = fork() ) == -1 )
		perror("fork");
	
	else if ( pid == 0 ) {
        
		signal( SIGINT, SIG_DFL );
		signal( SIGQUIT, SIG_DFL );

		if ( sym.pos != -1 && sym.symbol == '<' ) {
			
			close(0);
			
			int fd = open( argv[ sym.pos + 1 ], O_RDONLY );
			if ( fd < 0 ) { 
				fprintf( stderr, "Cannot open file\n");
				exit(1);
			}
			argv[ sym.pos ] = NULL;
		}

		if ( sym.pos != -1 && sym.symbol == '>' ) {
		
			close(1);
			
			int fd = open( argv[ sym.pos + 1 ], O_WRONLY | O_CREAT , 0666 );     
                        if ( fd < 0 ) {
                                fprintf( stderr, "Cannot open file\n");
                                exit(1);
                        }
			
			argv[ sym.pos ] = NULL;
                }

		if ( sym.pos != -1 && sym.symbol == '|' ) {

			int pid1;
			int thepipe[2];

			if ( pipe( thepipe ) == -1 ) {
				perror( "pipe" );
				exit(1);
			}

			if ( ( pid1 = fork() ) == -1 )
                		perror("fork");

        		if ( pid1 == 0 ) {
				
				close( thepipe[0] );
				
				if ( dup2( thepipe[1], 1 ) == -1 ) {
					perror( "dup2" );
					exit(1);
				}

				close( thepipe[1] );
	
				argv[ sym.pos ] = NULL;
				execvp( argv[0], argv );
				perror ( "cannot execute command" );
				exit(1);
			}	
			else {
				close( thepipe[1] );

				if ( dup2( thepipe[0], 0 ) == -1 ) {
					perror( "dup2" );
					exit(1);
				}
				
				close( thepipe[0] );
				
				if ( wait( NULL ) == -1 )
					perror( "wait" );
				argv[1] = NULL;
				execvp( argv[ sym.pos + 1 ], argv );
			}
		}
		else {
        
			execvp( argv[0], argv );
			perror ( "cannot execute command" );
			exit(1);
		}
	}
    
	else {
        
		if( wait( &childInfo ) == -1 )
			perror( "wait" );
        freeList( argv );
        free( comand );
	}
	
	return childInfo;
}

int main() {
    
	char *comand, **arglist;
	int result;
    
	setup();
    
	while ( ( comand = nextComand( stdin ) ) != NULL ) {
        
        result = execute( arglist, comand );
        
	}
	return 0;
}

 
