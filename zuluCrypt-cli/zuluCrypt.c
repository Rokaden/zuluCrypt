/*
 * 
 *  Copyright (c) 2011
 *  name : mhogo mchungu 
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include "zuluCrypt.h"
#include <unistd.h>
#include "String.h"
#include <fcntl.h>

#include "../version.h"

StrHandle * get_passphrase( void )
{	
	
	//I got the big chunk of this code from: http://www.gnu.org/s/hello/manual/libc/getpass.html
	char c[2] ;
	StrHandle * p ;
	
	struct termios old, new;
			
       /* Turn echoing off and fail if we can't. */
	if (tcgetattr (1, &old) != 0)
		exit(-1);

	new = old;
	new.c_lflag &= ~ECHO;
			
	if (tcsetattr (1, TCSAFLUSH, &new) != 0)
		exit(-1);
		
	c[1] = '\0' ;
	c[0] = getchar() ;
	
	p = StringCpy( c ) ;
	
	while( ( c[0] = getchar() ) != '\n' ){
		
		StringCat( p, c ) ;
	}
	
	(void) tcsetattr (1, TCSAFLUSH, &old);	
	
	return p ;
}

void help( void )
{
	printf("will write docs at some point\n");	
}

int volume_info( char * mapping_name )
{
	struct stat st ;
	char x[512] ;
	int Z ;
	
	StrHandle *s = StringCpy("/dev/mapper/zuluCrypt-") ;
	StringCat( s, mapping_name ) ;
	
	if( stat( StringCont( s ),&st) == 0 ){			
		status(mapping_name,x ,512 ) ;
		printf("%s",x);
		Z = 0 ;
	}else{
		printf("No opened volume associated with provided name\n") ;
		Z = 1 ;
	}
	
	StringDelete( s ) ;
	
	return Z ;	
}

int close_opened_volume( char * mapping_name )
{	
	int st = close_volume( mapping_name ) ;
	switch( st ) {
	case 0 : printf("SUCCESS: volume successfully closed\n");
		break ;
	case 1 : printf("ERROR: close failed, encrypted volume with that name does not exist\n");
		break ;
			
	case 2 : printf("ERROR: close failed, the mount point and/or one or more files are in use\n");
		break ;
			
	default :
		; //shouldnt get here			
	}	
	return st ;	
}

int open_volumes(int argn, char * device, char * mapping_name,int id, char * mount_point, char * mode,char *source, char * pass)
{
	StrHandle * p ;
	int st ;
	int f ;
	struct stat Q ;
	char *c ;

	if (strncmp(mount_point,",\0",2)==0){
			
		printf("ERROR, \",\"(comma) is not a valid mount point\n") ;
		return 6 ;			
	}		

	if (strncmp(mode,"ro",2) != 0){
		if (strncmp(mode,"rw",2) != 0){
			printf("ERROR: Wrong mode, run zuluCrypt with \"-h\" for help\n");;
			return 7 ;	
		}
	}		

	if (strncmp(mount_point,"-\0",2) ==0 ){
		mount_point = NULL ;
	}
	
	if ( argn == 5 ){
		printf( "Enter passphrase: " ) ;

		p = get_passphrase();				
		
		printf("\n") ;	
		
		st = open_volume(device, mapping_name,mount_point,id,mode,StringCont( p )) ;
		StringDelete( p ) ;
		
	}else if ( argn == 7 ){

		if( strcmp(source,"-p") == 0 ){
			
			st = open_volume(device, mapping_name,mount_point,id,mode, pass) ;
		
		}else if ( strcmp(source,"-f") == 0 ){
			
			if ( stat(pass,&Q) == 0 ){			
				
				c = ( char * ) malloc( sizeof(char) *  Q.st_size  ) ;
			
				f = open( pass,O_RDONLY ) ;
			
				read(f,c,Q.st_size) ;
				
				close(f);
				
				st = open_volume(device, mapping_name,mount_point,id,mode,c) ;
				
				free( c ) ;
			}else{

				st = 6 ;
			}
		}else{
			
			st = 7 ;
		}
	}
	else{
		
		st =  8 ;			
	}

	switch ( st ){
			
		case 0 : printf("SUCCESS: Volume opened successfully\n");
			break ;			
		case 1 : printf("ERROR: No free loop device to use\n") ; 
			break ;					
		case 2 : printf("ERROR: There seem to be an open volume accociated with given address\n");
			break ;				
		case 3 : printf("ERROR: No file exist on given path\n") ; 
			break ;		
		case 4 : printf("ERROR: Wrong passphrase\n");
			break ;			
		case 5 : printf("ERROR: a file or folder already exist at mount point\n") ;
			break ;		
		case 6 : printf("ERROR: passphrase file does not exist\n");
			break ;		
		case 7 : printf("ERROR: Wrong option, run zuluCrypt with \"-h\" for help\n");
			break ;
		case 8 : printf("ERROR: Wrong number of arguments, run zuluCrypt with \"-h\" for help\n");
			break ;
		default :
			;			
	}	
	return st ;
}


int create_volumes(int argn ,char *device, char *fs, char * mode, char * pass )
{
	StrHandle * p ;
	StrHandle * q ;
	char x[512] ;
	char Y ;
	int st ;
	
	if( realpath(device, x ) == NULL )
		return 4 ;		
		
	if(strncmp(x,"/dev/",5) == 0 ){
		printf("currently not supporting creating volumes on hard drives.\n") ;
		printf("This is an suid program and without any checks,this functionality\nwill enable ");
		printf("normal users to effectively delete a partition like roots.\n");
			
		return 6 ;
	}
			 
	if( argn == 5 ){
		printf("ARE YOU SURE YOU WANT TO CREATE/OVERWRITE: \"%s\" ? Type \"Y\" if you are\n",device);
		
		scanf("%c",&Y);
		
		if ( Y != 'Y')
			st = 5 ;
		else{			
			getchar();    //get rid of "\n" still in stdin buffer	
			
			printf("Enter passphrase: ") ;

			p = get_passphrase();
			
			printf("\nRe enter passphrase: ") ;

			q = get_passphrase();	
			
			printf("\n") ;
			
			if(strcmp(StringCont( p ),StringCont( q )) != 0){

				st = 3 ;
			}else{
				st = create_volume(device,fs,mode,StringCont( p )) ;
				StringDelete( q ) ;
				StringDelete( p ) ;			
			}
		}
			
	}else if ( argn == 6 ){
		
		st = create_volume(device,fs,mode,pass) ;
		
	}else{
		st = 4 ;			
	}
	
	switch ( st ){
		case 0 : printf("SUCCESS: volume successfully created\n") ;
			break  ;
		case 1 : printf("ERROR: File path given does not point to a file or partition\n") ;
			break  ;
		case 2 : printf("ERROR: Unrecognized volume type.\n");
			break  ;
		case 3 : printf("ERROR: passphrases do not match\n") ;
			break  ;
		case 4 : printf("ERROR: Wrong number of arguments\n");
			break  ;	
		case 5 : printf("ERROR: Wrong choice, exiting\n");
			break  ;
		default:
			;
	}	
	return st ;
}

void delete_file( char * file )
{
	int i, j  ;
	struct stat st ;	
	char X = 'X' ;
	
	stat(file, &st) ;

	i = open( file, O_WRONLY ) ;
	
	for( j = 0 ; j < st.st_size ; j ++ )
		write( i , &X, 1 ) ;				
	
	close( i ) ;
	
	remove( file ) ;	
}

int addkey(int argn,char * device, char *keyType1, char * existingKey, char * keyType2, char * newKey)
{
	StrHandle * p ;
	StrHandle * q ;
	StrHandle * n ;
	
	struct stat st1 ;
	int status = 0 ;
	int z ;
	char * c ;
	
	if ( argn == 3 ){		
		
		printf("Enter an existing passphrase: ") ;
		
		p = get_passphrase() ;
		
		printf("\n") ;				
			
		printf("Enter the new passphrase: ") ;
		
		q = get_passphrase() ;
		
		printf("\n") ;	
			
		printf("Re enter the new passphrase: ") ;
		
		n = get_passphrase() ;
		
		printf("\n") ;
		
		if (strcmp( StringCont( q ), StringCont( n ) ) != 0){
			
			status = 1 ;
		}else{
		
			z = open("/tmp/zuluCrypt-tmp",O_WRONLY | O_CREAT | O_TRUNC ) ;

			chown("/tmp/zuluCrypt-tmp",0,0) ;
			chmod("/tmp/zuluCrypt-tmp",S_IRWXU) ;
		
			write(z,StringCont( q ),strlen(StringCont( q ))) ;
		
			close( z ) ;
			
			status = add_key( device,StringCont( p ), "/tmp/zuluCrypt-tmp" ) ;
			
			delete_file("/tmp/zuluCrypt-tmp") ;				

			StringDelete( p ) ;			
			StringDelete( q ) ;	
			StringDelete( n ) ;	
		}
	}else if( argn == 7 ){			
		
		if ( strcmp( keyType1, "-f" ) == 0 ){			

			if( stat( existingKey, &st1) == 0 ) {
			
				c = ( char *) malloc ( sizeof(char) * st1.st_size ) ;
			
				z = open(existingKey, O_RDONLY ) ;
			
				read( z, c, st1.st_size ) ;
			
				close( z ) ;
			}else{
				status = 3 ;
				goto ouch ;
			}
		}
		
		if ( strcmp( keyType2, "-p" ) == 0){			
			
			z = open("/tmp/zuluCrypt-tmp",O_WRONLY | O_CREAT | O_TRUNC ) ;
			
			chown("/tmp/zuluCrypt-tmp",0,0) ;
			chmod("/tmp/zuluCrypt-tmp",S_IRWXU) ;

			write( z,newKey,strlen(newKey)) ;
		
			close( z ) ;		
		}
		
		if ( strcmp(keyType1,"-f") == 0 && strcmp(keyType2,"-f") == 0 ){
			
			status = add_key( device, c, newKey) ;
			
			free( c ) ;
			
		}else if (strcmp(keyType1,"-p") == 0 && strcmp(keyType2,"-p") == 0 ){
			
			status = add_key(device, existingKey, "/tmp/zuluCrypt-tmp" ) ;
			
			delete_file("/tmp/zuluCrypt-tmp") ;	
			
		}else if (strcmp(keyType1,"-p") == 0 && strcmp(keyType2,"-f") == 0 ){
			
			status = add_key( device, existingKey, newKey) ;	
			
		}else if (strcmp(keyType1,"-f") == 0 && strcmp(keyType2,"-p") == 0 ){			
		
			status = add_key( device, c, "/tmp/zuluCrypt-tmp") ;	
			
			delete_file("/tmp/zuluCrypt-tmp") ;	
	
			free( c ) ;
		}else{			
			status = 5 ;
		}
	}else{
		status = 6 ;		
	}
	
	ouch:
	
	switch ( status ){
		case 0 : printf("SUCCESS: key added successfully\n");
		break ;
		case 1 : printf("ERROR: new passphrases do not match\n") ;
		break ;
		case 2 : printf("ERROR: presented key does not match any key in the volume\n") ;
		break ;
		case 3 : printf("ERROR: key file containing a key in the volume does not exist\n") ;
		break ;  
		case 4 : printf("ERROR: device does not exist\n");
		break ;
		case 5 : printf("ERROR: Wrong arguments\n") ;
		break ;
		case 6 : printf("ERROR: Wrong number of arguments\n") ;
		break ;
	
		default :
			;		
	}
	return status ;
}

int killslot(int argn, char * device, char * keyType, char * existingkey, char * s)
{
	int status, i, d ;
	char * c ;
	struct stat st ;
	StrHandle * p ;
	
	int slotNumber = s[0]  ;
	
	if ( argn == 3 ){
		
		printf("Enter an existing passphrase: ") ;
		
		p = get_passphrase() ;
		
		printf("\n") ;
		
		printf("Enter a slot number to remove a key on: ") ;
		
		d = ( char ) getchar() ;
		
		getchar() ; //remove the new line character from stdin buffer
		
		status = kill_slot( device, StringCont( p ), d ) ;
		
		StringDelete( p ) ;
		
	}else if ( argn == 6 ){
	
		if( strcmp( keyType, "-p" ) == 0 ){
		
			status =  kill_slot(device, existingkey, slotNumber ) ;
	
		}else if ( strcmp( keyType, "-f" ) == 0 ){
		
			if ( stat( existingkey,&st ) != 0 ){
				return 4 ;
			}
		
			c = ( char * ) malloc ( sizeof( char ) * st.st_size ) ;
		
			i = open( existingkey, O_RDONLY ) ;
		
			write( i , c , st.st_size ) ;
		
			close( i ) ;
		
			status = kill_slot( device, c , slotNumber ) ;
		
			free( c ) ;				
			
		}else{
			status = 5 ;		
		}
	}else{		
		status = 6 ;		
	}	
	switch ( status ){
		case 0 : printf("SUCCESS: slot successfully killed\n");
		break ;
		case 1 : printf("ERROR: slot to be killed is inactive/empty\n") ;
		break ;
		case 2 : printf("ERROR: the device does not exist\n") ;
		break ;
		case 3 : printf("ERROR: presented key does not match any key in the volume\n") ;
		break ;  
		case 4 : printf("ERROR: key file does not exist\n");
		break ;
		case 5 : printf("ERROR: Wrong arguments\n") ;
		break ;
		case 6 : printf("ERROR: Wrong number of arguments\n") ;
		break ;
		default :
			;		
	}	
	return status ;
}

int removekey( int argn , char * device, char * keyType, char * keytoremove )
{
	StrHandle *p;
	int status, z ;
	struct stat st ;
	
	if ( argn == 3 ){
		
		printf("Enter the passphrase of the key you want to delete: ") ;
		
		p = get_passphrase() ;
		
		printf("\n") ;
		
		z = open("/tmp/zuluCrypt-tmp",O_WRONLY | O_CREAT | O_TRUNC ) ;
			
		chown("/tmp/zuluCrypt-tmp",0,0) ;
		chmod("/tmp/zuluCrypt-tmp",S_IRWXU) ;

		write( z, StringCont( p ) ,StringLength( p )) ;
		
		close( z ) ;
		
		status = remove_key( device,"/tmp/zuluCrypt-tmp" ) ;
		
		StringDelete( p ) ;
			
		delete_file("/tmp/zuluCrypt-tmp");
		
	}else if ( argn == 5 ){
		
		if( strcmp(keyType, "-f") == 0 ){
			
			if ( stat(keytoremove,&st) == 0 )
				status = remove_key(device, keytoremove );
			else
				status = 5 ;
			
		}else if( strcmp(keyType, "-p") == 0 ) {
			
			z = open("/tmp/zuluCrypt-tmp",O_WRONLY | O_CREAT | O_TRUNC ) ;
			
			chown("/tmp/zuluCrypt-tmp",0,0) ;
			chmod("/tmp/zuluCrypt-tmp",S_IRWXU) ;

			write( z, keytoremove ,strlen(keytoremove)) ;
		
			close( z ) ;
		
			status = remove_key( device,"/tmp/zuluCrypt-tmp" ) ;
		
			delete_file("/tmp/zuluCrypt-tmp");			
		}else
			status = 6 ;
	}
	switch ( status ){
		case 0 : printf("SUCCESS: key successfully removed\n");
		break ;
		//case 1 : printf("") ;
		//break ;
		case 2 : printf("ERROR: there is no key in the volume that match the presented key\n") ;
		break ;
		//case 3 : printf("") ;
		//break ;  
		case 4 : printf("ERROR: device does not exist\n");
		break ;
		case 5 : printf("ERROR: keyfile does not exist\n") ;
		break ;
		case 6 : printf("ERROR: Wrong number of arguments\n") ;
		break ;
		default :
			;		
	}		
	return status ;	
}

int main( int argc , char *argv[])
{
	char * action = argv[1] ;
	char * device = argv[2] ;

	uid_t id ;
	
	char *  mapping_name ;
	char * c ;
	
	char slots[7] = {'0','0','0','0','0','0','0'};;
	
	id = getuid();	
	
	setuid(0);
	
	if ( strcmp( action, "-h" ) == 0 || strcmp( action, "--help" ) == 0 || strcmp( action, "-help" ) == 0 ){			
		help();	
		return 10 ;
	}
	
	if ( strcmp( action, "-v" ) == 0 || strcmp( action, "-version" ) == 0 || strcmp( action, "--version" ) == 0 ){		
		printf(VERSION_STRING) ;
		printf("\n");
		return 10 ;
	}		
		
	if ( argc < 3 ){
		help();
		return 10 ;
	}
		
	if ( (c = strrchr(device,'/')) != NULL) {
		mapping_name =  c + 1  ;	
		
	}else{
		mapping_name =  device  ;			
	}		
	
	if( strcmp( action, "isLuks" ) == 0 ){
	
		return is_luks( device ) ;
		
	}else if ( strcmp( action, "status" ) == 0 ){			

		return volume_info( mapping_name ) ;
		
	}else if ( strcmp( action, "close" ) == 0 ){			

		return close_opened_volume( mapping_name ) ;
		
	}else if ( strcmp( action, "open" ) == 0 ){
		
		return open_volumes(argc,argv[2],mapping_name,id,argv[3],argv[4],argv[5],argv[6] ) ;		
		
	}else if(strcmp(action,"create") == 0 ){

		return create_volumes(argc ,argv[2],argv[3],argv[4],argv[5] ) ;	
		
	}else if(strcmp(action,"addkey") == 0 ){
		
		return addkey(argc,argv[2],argv[3],argv[4],argv[5],argv[6]) ;
		
	}else if(strcmp(action,"killslot") == 0 ){
		
		return killslot(argc, argv[2],argv[3],argv[4],argv[5] ) ;
	
	}else if(strcmp(action,"removekey") == 0 ){
				
		return removekey(argc, argv[2], argv[3],argv[4] );
		
	}else if(strcmp(action,"emptyslots") == 0 ){
		
		if ( empty_slots( slots , argv[2] ) == 0 ){			
			printf("%s\n",slots ) ;
			return 0 ;
		}else{
			printf("ERROR: given device does not exist\n") ;
			return 1 ;
		}
			
	}else{
		printf("ERROR: Wrong argument\n") ;
		return 10 ;
	}
	
	return 0 ; //shouldnt get here		
}