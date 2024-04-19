#include <stddef.h>
#include "library/syscalls.h"
#include "named_pipe.h"
#include "kmalloc.h"
#include "string.h"

static struct named_pipe_mapping named_pipe_map[MAX_NAMED_PIPES];
static int find_free_mapping_index(void);
#define MAX_PATH_LEN 100


static void getParentPath(const char *filePath, char *parentPath) {
    strncpy(parentPath, filePath, MAX_PATH_LEN-1);  // Copy the input to the output buffer

    // Find the last occurrence of '/'
    char *lastSlash = strchr(parentPath, '/');
    if (lastSlash != NULL) {
        *lastSlash = '\0';  // Truncate the path
    } else {
        // No slash found, return an error or handle the case as needed
        strncpy(parentPath, ".",MAX_PATH_LEN-1);  // Assume the current directory if no parent
    }
}

static void getFilename(const char *filePath, char *filename) {
    const char *lastSlash = filePath;
    while (*filePath) {
        if (*filePath == '/') {
            lastSlash = filePath + 1; // Move just past the '/'
        }
        filePath++;
    }
    strncpy(filename, lastSlash, MAX_PATH_LEN - 1);
}

static void extract_parentpath_and_file(const char *path, char *parentPath, char *fileName) {
    getParentPath(path,parentPath);
    getFilename(path,fileName);
}


struct fs_dirent * createfile( const char *path ){

    char parentPath[MAX_PATH_LEN];
    char fileName[MAX_PATH_LEN];

    // Get parent dit and file name
    extract_parentpath_and_file(path, parentPath, fileName);

    // Get parent dirent
    struct fs_dirent *parentDir = fs_resolve(parentPath);
    if (!parentDir) {
        printf("named_pipe_create: Fail to get parent dirent\n");
        return NULL; 
    }

    // Create file 
    struct fs_dirent *file = fs_dirent_mkfile(parentDir, fileName);
    fs_dirent_close(parentDir);  // no need, so close
    if (!file) {
        printf("named_pipe_create: Fail to create File\n");
        return NULL; // pipe dirent creation failed
    }
    return file;
}

// Finds a empty entry to map file and pipe */
static int find_free_mapping_index() {
    for (int i = 0; i < MAX_NAMED_PIPES; i++) {
        if (named_pipe_map[i].named_pipe == NULL) {
            return i;
        }
    }
    printf("Couldn't find_free_mapping_index");
    return -1;
}

// Map pipe and file
static int map_named_pipe(struct named_pipe *named_pipe, struct fs_dirent *file) {
    int index = find_free_mapping_index();
    if (index >= 0) {
        named_pipe_map[index].named_pipe = named_pipe;
        named_pipe_map[index].file = file;
    }
}

int named_pipe_create(const char *fname) {
    // create file
    struct fs_dirent *file = createfile(fname);

    // struct pipe *p = kmalloc(sizeof(*p));
	// if(!p) return 0;
    struct named_pipe *np = kmalloc(sizeof(*np));
    if(!np) return -1;

    // create pipe
    np->base_pipe = pipe_create(); 
    if (!np->base_pipe) return -1;

    // assign path
    np->path = strdup(fname);
    if (!np->path) return -1;

    // map file and pipe
    map_named_pipe(np, file);

    return 0;
}

// Find the named pipe by path
static struct named_pipe *find_named_pipe_by_path(const char *path) {
    for (int i = 0; i < MAX_NAMED_PIPES; i++) {
        if (named_pipe_map[i].named_pipe != NULL) {
            if (strcmp(named_pipe_map[i].named_pipe->path, path) == 0) { 
                    return named_pipe_map[i].named_pipe;
            }
        }
    }
    return NULL; 
}

int named_pipe_open(const char *path, struct named_pipe **named_pipe) {
    struct named_pipe *np = find_named_pipe_by_path(path);
    if (!np) {
        printf("Np not found: %s.\n", path);
        return -1;
    }
    *named_pipe = np; 
    return 0; // success
}