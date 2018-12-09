#include "library/validate.h"
#include "library/string.h"

int main(const char *argv[], int argc) {
	printf("Testing PATH validations...\n");
	char * s = "/root/is-valid.path";
	if (!is_valid_path(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "abcdefghijklmnopqrstuvwxyz";
	if (!is_valid_path(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (!is_valid_path(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "0123456789";
	if (!is_valid_path(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "/root/isn't/valid:";
	if (is_valid_path(s))
		printf("ERROR: Did not invalidate %s\n", s);

	printf("Testing Tag Validations...\n");
	s = "TAG";
	if (!is_valid_tag(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "abcdefghijklmnopqrstuvwxyz";
	if (!is_valid_tag(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (!is_valid_tag(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "0123456789";
	if (!is_valid_tag(s))
		printf("ERROR: Did not validate %s\n", s);
	s = ":NOT-A-TAG";
	if (is_valid_tag(s))
		printf("ERROR: Did not invalidate %s\n", s);

	printf("Testing Location Validations...\n");
	s = "HOME:path/to/root/dir";
	if (!is_valid_location(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "TAG";
	if (!is_valid_tag(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "/root/is-valid.path";
	if (!is_valid_path(s))
		printf("ERROR: Did not validate %s\n", s);
	s = "BAD-INTENT:p@th/:/root/dir";
	if (is_valid_location(s))
		printf("ERROR: Did not invalidate %s\n", s);

	return 0;
}
