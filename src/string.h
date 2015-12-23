/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef STRING_H
#define STRING_H

void	 strcpy( char *d, const char *s );
int	 strcmp( const char *a, const char *b );
int	 strncmp( const char *a, const char *b, unsigned length );
unsigned strlen( const char *s );

char * strtok( char *s, const char *delim );

const char * strchr( const char *s, char ch );

void	memset( void *d, char value, unsigned length );
void	memcpy( void *d, const void *s, unsigned length );

void printf( const char *s, ... );

#endif
