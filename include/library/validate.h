#ifndef VALIDATE_H
#define VALIDATE_H

// Does this string comprise a valid path?
int is_valid_path(const char * test_string);
// Does this string comprise a valid intent? 
int is_valid_tag(const char * test_string);

// Does this string point to a valid resource (i.e. is it a legitimate
// intent-path concatenation)?
int is_valid_location(const char * test_string);

#endif
