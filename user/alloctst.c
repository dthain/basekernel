/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
Test to check that we are able to allocate more than 1 page of memory on the stack
*/

#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"
#include "library/malloc.h"

int func1();

// int main() {
// 	// // Test 1: Check if we can allocate a large 2D array on the stack
	// int t_size = 500, i, j;
	// int test_1_array_1 [t_size][t_size];
	// int test_1_array_2 [t_size][t_size];
	// for (i = 0; i < t_size; ++i) {
	// 	for (j = 0; j < t_size; ++j) {
	// 		test_1_array_1[i][j] = i%10; 
	// 		test_1_array_2[i][j] = i%15; 
	// 	}
	// }
	// printf_putstring("Test 1 passed!\n");
	// flush();


	// // Test 2: Check that allocating on stack does not interfere
	// int test_2_array_1[10000];
	// int * test_2_array_2 = malloc(10000);

	// for (i = 0; i < 10000; ++i)
	// {
	// 	test_2_array_1[i] = i%10;
	// 	test_2_array_2[i] = i%15;
	// }

	// free(test_2_array_2);

	// printf_putstring("Test 2 passed!\n");
	// flush();	


	// // Test 3: Make sure calls to other functions works
	// func1();
	// int test_3_array[100000];
	// for (i = 0; i < 100000; ++i) {
	// 	test_3_array[i] = i%10;
	// }
	
	// printf_putstring("Test 3 passed!\n");
	// flush();	

	// Test 4: Test for failure of malloc
int main() {
	func1();
	// printf("Stack address %x\n");
	unsigned sp;
	asm( "mov %%esp, %0" : "=rm" ( sp ));
	int * temp;

	temp = (int *)((void *)sp - 5000);
	temp[0] = 1;
	
	printf("Stack address %x\n", sp);
	printf("Temp  address %x\n", &temp[0]);



	printf_putstring("Fail passed!\n");
	flush();
	return 0;
}


int func1() {
	int t_size = 500, i, j;
	int test_1_array_1 [t_size][t_size];
	int test_1_array_2 [t_size][t_size];
	for (i = 0; i < t_size; ++i) {
		for (j = 0; j < t_size; ++j) {
			test_1_array_1[i][j] = i%10; 
			test_1_array_2[i][j] = i%15; 
		}
	}
	int r[1000];
	r[0] = 1;
	r[100] = 10;
	r[500] = 6;
	return r[0];
}






// int main1() {
// 	unsigned sp1;
// 	int t[5000];
// 	asm( "mov %%esp, %0" : "=rm" ( sp1 ));
// 	printf("%x\n", sp1);

// 	// Allocating variables just moves the stack pointer
// 	//int * temp;
// 	// int t[6000];
// 	// asm( "mov %%esp, %0" : "=rm" ( sp2 ));
	
// 	// if (sp1 == sp2) {
// 	// 	printf("SP: %x\n", sp2);
// 	// } else {
// 	// 	printf("SP1: %x\nSP2: %x\n", sp1, sp2);
// 	// }

	
// 	// func1();
// 	// temp = (int *)((void *)t - 1);



// 	// t[1] = 1;
// 	// unsigned i = 0x80000000;

	

// 	// int * temp;

// 	// asm( "mov %%esp, %0" : "=rm" ( sp ));

// 	return 0;

// 	//i = sp;

// 	// while(1) {
// 	// 	temp = (int *)((void *)i);
// 	// 	temp[0] = 1;
// 	// 	i++;
// 	// }

// }