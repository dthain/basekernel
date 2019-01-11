/*
 * validate.c
 * Containts logic for validating strings pointing to resources in the fs.
 */

#include "is_valid.h"
#include "string.h"
#include "kobject.h"
#include "process.h"
#include "kmalloc.h"

// Does this string comprise a valid path?
// Valid paths are comprised of the following characters:
// CHARS		|	ASCII CODE
// '-'			|	45
// '.'			|	46
// '/'			|	47
// [0-9]		|	[48-57]
// [a-z]		|	[65-90]
// '_'			|	95
// [A-Z]		|	[97-122]

int is_valid_path(const char * s)
{
	int length = strlen(s);
	for (int i = 0; i < length; i++) {
		if (s[i] < 45) return 0;
		if (s[i] > 57 && s[i] < 65) return 0;
		if (s[i] > 90 && s[i] < 95) return 0;
		if (s[i] > 95 && s[i] < 97) return 0;
		if (s[i] > 122) return 0;
	}
	return 1;
}

// Does this string comprise a valid tag? 
// CHARS		|	ASCII CODE
// [0-9]		|	[48-57]
// [a-z]		|	[65-90]
// [A-Z]		|	[97-122]

int is_valid_tag(const char * s)
{
	int length = strlen(s);
	for (int i = 0; i < length; i++) {
		if (s[i] < 48) return 0;
		if (s[i] > 57 && s[i] < 65) return 0;
		if (s[i] > 90 && s[i] < 95) return 0;
		if (s[i] > 95 && s[i] < 97) return 0;
		if (s[i] > 122) return 0;
	}
	return 1;
}

// Does this string point to a valid resource (i.e. is it a legitimate
// intent-path concatenation)?
// CHARS		|	ASCII CODE
// ':'			|	58

int is_valid_location(const char * s)
{
	int length = strlen(s);
	char * mutable_test = kmalloc(sizeof(char) * (length+1));
	strcpy(mutable_test, s);

	int path_index = 0;
	while (path_index < length && s[path_index] != 58) {
		path_index += 1;
	}
	mutable_test[path_index++] = 0;

	if (is_valid_tag(mutable_test)
		&& is_valid_path(mutable_test + path_index)) {
		return 1;
	}
	return 0;
}

// Return true if file desciptor is in range and refers to a live object.
int is_valid_object( int fd )
{
	return fd>=0 && fd<PROCESS_MAX_OBJECTS && current->ktable[fd];
}

// Return true if fd valid and object is also of indicated type.
int is_valid_object_type( int fd, kobject_type_t type )
{
	return is_valid_object(fd) && kobject_get_type(current->ktable[fd])==type;
}

// Return true if (ptr,length) describes a valid area in user space.
// XXX Needs to be implemented!

int is_valid_pointer( void *ptr, int length )
{
	return 1;
}

// Return true if string points to a valid area in user space.
// XXX Needs to be implemented!

int is_valid_string( const char *str )
{
	return 1;
}

