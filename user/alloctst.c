/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
Test to check that we are able to allocate more than 1 page of memory on the stack
*/

#include "library/syscalls.h"
#include "library/user-io.h"

int main() {
	// Test 1: Check if we can allocate a large 2D array on the stack
	int t_size = 500;
	int test_1_array [t_size][t_size];
	for (int i = 0; i < t_size; ++i) {
		for (int j = 0; j < t_size; ++j) {
			test_1_array[i][j] = 19; 
		}
	}
	printf_putstring("Test 1 passed!\n");
	flush();

	// Test 2: Check that allocating large amounts on the heap and stack
	// simultaneously does not cause any errors
	char * test_2_string = malloc(5000);
	int test_2_array[100000];
	test_2_string = "test 2";
	test_2_array[17632] = 4;
	printf_putstring("Test 2 passed!\n");
	flush();

	return 0;
}