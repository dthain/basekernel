/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/types.h"
#include "named_pipe.h"
#include "kmalloc.h"
#include "process.h"
#include "page.h"
#include "named_pipe.h"
//
#define MAX_PATH_LEN 100
#include <stddef.h>
//

void seperate_parentdir_filename(const char *path, char *parentdir, char *fileName) {
    const char *lastSlash = path;
    const char *current = strchr(path, '/');

    int pathsize = MAX_PATH_LEN;

    while (current != NULL) {
        lastSlash = current;
        current = strchr(current + 1, '/');
    }

    // If no slash was found, or the fullPath is the root directory
    if (lastSlash == path) {
        // Handle special case, such as setting parentPathBuffer to "/"
        strncpy(parentdir, "/", pathsize - 1);
        parentdir[pathsize - 1] = '\0';
        fileName[0] = '\0'; // No file name in this case
    } else {
        int parentLength = lastSlash - path;
        if (parentLength >= pathsize) parentLength = pathsize - 1;

        strncpy(parentdir, path, parentLength);
        parentdir[parentLength] = '\0';

        strncpy(fileName, lastSlash + 1, pathsize - 1);
        fileName[pathsize - 1] = '\0';
    }
}


struct fs_dirent * createfile( const char *path ){

    char parentPath[MAX_PATH_LEN] = {0};
    char fileName[MAX_PATH_LEN] = {0};
    seperate_parentdir_filename(path, parentPath, fileName);

    // Get dirent for parent dir
    struct fs_dirent *parentDir = fs_resolve(parentPath);
    if (!parentDir) {
        // printf("Get parent dir fail");
        return NULL; 
    }

    // Create file 
    struct fs_dirent *file = fs_dirent_mkfile(parentDir, fileName);
    fs_dirent_close(parentDir);  // no need, so close
    if (!file) {
        // printf("Create File Fail");
        return NULL; // pipe dirent creation failed
    }
    return file;
}

#define NAMED_PIPE_SIZE PAGE_SIZE

struct named_pipe
{
    char *fname;
    char *buffer;
    int read_pos;
    int write_pos;
    int flushed;
    int refcount;
    struct list queue;
};

struct named_pipe *named_pipe_create(char *fname)
{
    struct fs_dirent * file =  createfile(fname);
    //
    struct named_pipe *p = kmalloc(sizeof(*p));
    if (!p)
        return 0;

    p->buffer = page_alloc(1);
    if (!p->buffer)
    {
        kfree(p);
        return 0;
    }
    p->fname = fname;
    p->read_pos = 0;
    p->write_pos = 0;
    p->flushed = 0;
    p->queue.head = 0;
    p->queue.tail = 0;
    p->refcount = 1;
    return p;
}

struct named_pipe *named_pipe_addref(struct named_pipe *p)
{
    p->refcount++;
    return p;
}

void named_pipe_flush(struct named_pipe *p)
{
    if (p)
    {
        p->flushed = 1;
    }
}

void named_pipe_delete(struct named_pipe *p)
{
    if (!p)
        return;

    p->refcount--;
    if (p->refcount == 0)
    {
        if (p->buffer)
        {
            page_free(p->buffer);
        }
        kfree(p);
    }
}

static int named_pipe_write_internal(struct named_pipe *p, char *buffer, int size, int blocking)
{
    if (!p || !buffer)
    {
        return -1;
    }
    int written = 0;
    if (blocking)
    {
        for (written = 0; written < size; written++)
        {
            while ((p->write_pos + 1) % NAMED_PIPE_SIZE == p->read_pos)
            {
                if (p->flushed)
                {
                    p->flushed = 0;
                    return written;
                }
                process_wait(&p->queue);
            }
            p->buffer[p->write_pos] = buffer[written];
            p->write_pos = (p->write_pos + 1) % NAMED_PIPE_SIZE;
        }
        process_wakeup_all(&p->queue);
    }
    else
    {
        while (written < size && p->write_pos != (p->read_pos - 1) % NAMED_PIPE_SIZE)
        {
            p->buffer[p->write_pos] = buffer[written];
            p->write_pos = (p->write_pos + 1) % NAMED_PIPE_SIZE;
            written++;
        }
    }
    p->flushed = 0;
    return written;
}

int named_pipe_write(struct named_pipe *p, char *buffer, int size)
{
    return named_pipe_write_internal(p, buffer, size, 1);
}

int named_pipe_write_nonblock(struct named_pipe *p, char *buffer, int size)
{
    return named_pipe_write_internal(p, buffer, size, 0);
}

static int named_pipe_read_internal(struct named_pipe *p, char *buffer, int size, int blocking)
{
    if (!p || !buffer)
    {
        return -1;
    }
    int read = 0;
    if (blocking)
    {
        for (read = 0; read < size; read++)
        {
            while (p->write_pos == p->read_pos)
            {
                if (p->flushed)
                {
                    p->flushed = 0;
                    return read;
                }
                if (blocking == 0)
                {
                    return -1;
                }
                process_wait(&p->queue);
            }
            buffer[read] = p->buffer[p->read_pos];
            p->read_pos = (p->read_pos + 1) % NAMED_PIPE_SIZE;
        }
        process_wakeup_all(&p->queue);
    }
    else
    {
        while (read < size && p->read_pos != p->write_pos)
        {
            buffer[read] = p->buffer[p->read_pos];
            p->read_pos = (p->read_pos + 1) % NAMED_PIPE_SIZE;
            read++;
        }
    }
    p->flushed = 0;
    return read;
}

int named_pipe_read(struct named_pipe *p, char *buffer, int size)
{
    return named_pipe_read_internal(p, buffer, size, 1);
}

int named_pipe_read_nonblock(struct named_pipe *p, char *buffer, int size)
{
    return named_pipe_read_internal(p, buffer, size, 0);
}

int named_pipe_size(struct named_pipe *p)
{
    return NAMED_PIPE_SIZE;
}

// implimented by Mahir 

// int make_named_pipe(const char *name) {
//     named_pipe *pipe = kmalloc(sizeof(named_pipe));
//     if (!pipe)
//         return -ENOMEM;

//     strncpy(pipe->name, name, sizeof(pipe->name));
//     pipe->read_pos = 0;
//     pipe->write_pos = 0;
//     pipe->open_handles = 1;
//     mutex_init(&pipe->lock);

//     fs_dirent_mkfile(name, FS_CHARDEVICE, pipe);
//     return 0;
// }



// opening named pipes 


// int open_named_pipe(const char *name) {
//     struct file *file = fs_open(name, FS_READ | FS_WRITE);
//     if (!file)
//         return -ENOENT;

//     return file->fd;
// }



