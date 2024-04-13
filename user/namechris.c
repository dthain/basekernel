#include "library/syscalls.h"

int main(){
    char * fname = "/path/to/file/filename";
    int a = syscall_make_named_pipe(fname);
    int b = syscall_open_named_pipe(fname);
    return 0;
}
