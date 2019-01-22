/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef IS_VALID_H
#define IS_VALID_H

#include "kobject.h"

// Does this string comprise a valid path?
int is_valid_path(const char *s);

// Does this string comprise a valid tag? 
int is_valid_tag(const char *s);

// Does this string point to a valid resource (i.e. is it a legitimate
// tag-path concatenation)?
int is_valid_location(const char *s);

// Return true if file desciptor is in range and refers to a live object.
int is_valid_object( int fd );

// Return true if fd valid and object is also of indicated type.
int is_valid_object_type( int fd, kobject_type_t type );

// Return true if (ptr,length) describes a valid area in user space.
int is_valid_pointer( void *ptr, int length );

// Return true if string points to a valid area in user space.
int is_valid_string( const char *str );


#endif
