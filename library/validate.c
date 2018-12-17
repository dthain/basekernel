/*
 * validate.c
 * Containts logic for validating strings pointing to resources in the fs.
 */

#include "library/validate.h"
#include "library/malloc.h"
#include "library/string.h"

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
int is_valid_path(const char * test_string) {
	int max_length = strlen(test_string);
	for (int i = 0; i < max_length; i++) {
		if (test_string[i] < 45) return 0;
		if (test_string[i] > 57 && test_string[i] < 65) return 0;
		if (test_string[i] > 90 && test_string[i] < 95) return 0;
		if (test_string[i] > 95 && test_string[i] < 97) return 0;
		if (test_string[i] > 122) return 0;
	}
	return 1;
}

// Does this string comprise a valid tag? 
// CHARS		|	ASCII CODE
// [0-9]		|	[48-57]
// [a-z]		|	[65-90]
// [A-Z]		|	[97-122]
int is_valid_tag(const char * test_string) {
	int max_length = strlen(test_string);
	for (int i = 0; i < max_length; i++) {
		if (test_string[i] < 48) return 0;
		if (test_string[i] > 57 && test_string[i] < 65) return 0;
		if (test_string[i] > 90 && test_string[i] < 95) return 0;
		if (test_string[i] > 95 && test_string[i] < 97) return 0;
		if (test_string[i] > 122) return 0;
	}
	return 1;
}

// Does this string point to a valid resource (i.e. is it a legitimate
// intent-path concatenation)?
// CHARS		|	ASCII CODE
// ':'			|	58
int is_valid_location(const char * test_string) {
	int max_length = strlen(test_string);
	char * mutable_test = malloc(sizeof(char) * (max_length+1));
	strcpy(mutable_test, test_string);

	int path_index = 0;
	while (path_index < max_length && test_string[path_index] != 58) {
		path_index += 1;
	}
	mutable_test[path_index++] = 0;

	if (is_valid_tag(mutable_test)
		&& is_valid_path(mutable_test + path_index)) {
		return 1;
	}
	return 0;
}
