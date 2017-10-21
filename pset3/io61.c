#include "io61.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

// io61_file
// Data structure for io61 file wrappers.
struct io61_file {
    int fd;
    char buffer[4096];
    int fixed_start;
    size_t start;
    size_t end;
    size_t mode;
    size_t start_in_file;
    size_t end_in_file;
    size_t curr;
};
// io61_fdopen(fd, mode)
//    Return a new io61_file for file descriptor `fd`. `mode` is
//    either O_RDONLY for a read-only file or O_WRONLY for a
//    write-only file. You need not support read/write files.
io61_file* io61_fdopen(int fd, int mode) {
    assert(fd >= 0);
    io61_file* f = (io61_file*) malloc(sizeof(io61_file));
    f->fd = fd;
    f->start = 0;
    f->end = 4096;
    f->end_in_file = 0;
    f->start_in_file= 0;
    f->curr = 0;
    f->mode = mode;
    (void) mode;
    return f;
}
// io61_close(f)
//    Close the io61_file `f` and release all its resources.

int io61_close(io61_file* f) {
    if (f->start >0){
        write(f->fd, f->buffer, f->start);
    }
    io61_flush(f);
    int r = close(f->fd);
    free(f);
    return r;
}
// read the next 4096 bytes to file buffer which will be my buffer size
// might be slower for reverse since am reading forward
ssize_t read_to_file(io61_file* f){
    f->end= 4096;
    int m = read(f->fd,f->buffer,f->end);
    if (m > 0){
        return m;
    }
    else {
        return EOF;
    }
    
}
// io61_readc(f)
//    Read a single (unsigned) character from `f` and return it. Returns EOF
//    (which is -1) on error or end-of-file.
int io61_readc(io61_file* f) {
    unsigned char buf[1];
    int m;
    int sz = 1;
    if (f->curr >= f->start_in_file && f->curr< f->end_in_file){
        if (f->curr+sz > f->end_in_file){
            
            int left_over  = f->curr+sz-f->end_in_file;
            memcpy(buf,f->buffer+f->curr-f->start_in_file,f->end_in_file-f->curr);
            f->start =0;
            m = read_to_file(f);
            if (m>0 && m>=left_over){
                
                memcpy(buf+f->end_in_file-f->curr,f->buffer,left_over);
                f->start_in_file = f->end_in_file;
                f->end_in_file = f->end_in_file+m;
                f->curr = f->start_in_file+left_over;
                return buf[0];
            }
            else if(m>0 && m<left_over) {
                
                memcpy(buf+f->end_in_file-f->curr,f->buffer,m);
                f->start_in_file = f->end_in_file;
                f->end_in_file = f->end_in_file+m;
                f->curr = f->start_in_file+m;
                return buf[0];
            }
            else {
                   
                f->curr = f->end_in_file;
                return buf[0];
            }
            }
        else{
            memcpy(buf,f->buffer+f->curr-f->start_in_file,sz);
            f->curr = f->curr + sz;
            return buf[0];
        }
    }
    else{
        m = read_to_file(f);
        if (m>0 && m>=sz){
                
            memcpy(buf,f->buffer,sz);
            f->start_in_file = f->curr;
            f->end_in_file = f->curr+m;
            f->curr = f->curr+sz;
            return buf[0];
        }
        else if(m>0 && m<sz) {
                
            memcpy(buf,f->buffer,m);
            f->start_in_file = f->curr;
            f->end_in_file = f->curr+m;
            f->curr = f->curr+m;
            return buf[0];
        }
        else {
            return -1;
        }
            
    }
    
}
// io61_read(f, buf, sz)
//    Read up to `sz` characters from `f` into `buf`. Returns the number of
//    characters read on success; normally this is `sz`. Returns a short
//    count, which might be zero, if the file ended before `sz` characters
//    could be read. Returns -1 if an error occurred before any characters
//    were read.
ssize_t io61_read(io61_file* f, char* buf, size_t sz) {
    ssize_t m;
    if (f->curr >= f->start_in_file && f->curr< f->end_in_file){
        
        if (f->curr+sz > f->end_in_file){
            int left_over  = f->curr+sz-f->end_in_file;
            int remaining = f->end_in_file-f->curr;
            memcpy(buf,f->buffer+f->curr-f->start_in_file,f->end_in_file-f->curr);
            f->start =0;
            m = read_to_file(f);
            if (m>0 && m>=left_over){
                memcpy(buf+f->end_in_file-f->curr,f->buffer,left_over);
                f->start_in_file = f->end_in_file;
                f->end_in_file = f->end_in_file+m;
                f->curr = f->start_in_file+left_over;
                return remaining+left_over;
            }
            else if(m>0 && m<left_over) {
                memcpy(buf+f->end_in_file-f->curr,f->buffer,m);
                f->start_in_file = f->end_in_file;
                f->end_in_file = f->end_in_file+m;
                f->curr = f->start_in_file+m;
                return remaining+m;
            }
            else {
                f->curr = f->end_in_file;
                return remaining;
            }
    
        }
        else{
            memcpy(buf,f->buffer+f->curr-f->start_in_file,sz);
            f->curr = f->curr + sz;
            return sz;
        }
    }
    else{
        m = read_to_file(f);
        if (m>0 && m>=(ssize_t)sz){
            memcpy(buf,f->buffer,sz);
            f->start_in_file = f->curr;
            f->end_in_file = f->curr+m;
            f->curr = f->curr+sz;
            return sz;
        }
        else if(m>0 && m<(ssize_t)sz) {
            memcpy(buf,f->buffer,m);
            f->start_in_file = f->curr;
            f->end_in_file = f->curr+m;
            f->curr = f->curr+m;
            return m;
        }
        else{
            return -1;
        }

    }
    
}
// io61_writec(f)
//    Write a single character `ch` to `f`. Returns 0 on success or
//    -1 on error.
int io61_writec(io61_file* f, int ch) {
    unsigned char buf[1];
    if (f->start < f->end){
        f->buffer[f->start] = ch;
        f->start = f->start + 1;
        return buf[0];
    }
    else{
        int m = write(f->fd, f->buffer, 4096);
        if (m> 0){
            f->start = 1;
            f->buffer[0] = ch;
        }
        return m;
    }
}
// io61_write(f, buf, sz)
//    Write `sz` characters from `buf` to `f`. Returns the number of
//    characters written on success; normally this is `sz`. Returns -1 if
//    an error occurred before any characters were written.

ssize_t io61_write(io61_file* f, const char* buf, size_t sz) {
    if (f->curr>=f->start_in_file && f->curr<f->end_in_file && f->end_in_file - f->curr >= sz){
        
        memcpy(f->buffer+f->curr-f->start_in_file,buf,sz);
        f->curr = f->curr + sz;
        return sz;
    }
    
    else if( f->curr>=f->start_in_file && f->curr<f->end_in_file && f->end_in_file-f->curr<sz){
        
        memcpy(f->buffer+f->curr-f->start_in_file,buf,f->end_in_file-f->curr);
        int temp = f->curr;
        int left_over = f->curr+sz-f->end_in_file;
        write(f->fd,f->buffer,4096);
        memcpy(f->buffer,buf+f->end_in_file-temp,left_over);
        f->start_in_file = f->end_in_file;
        f->end_in_file = f->end_in_file+4096;
        f->start  = 0;
        f->curr = f->start_in_file + left_over;
        return sz;
    }
    else{
        f->start = 0;
        if (f->curr>0 && f->curr ==f->end_in_file){
        write(f->fd,f->buffer,4096);
        }
        f->start_in_file = f->curr;
        f->end_in_file = f->curr+4096;
        f->curr = f->start_in_file + sz;
        memcpy(f->buffer,buf,sz);
        return sz;
    }
}


// io61_flush(f)
//    Forces a write of all buffered data written to `f`.
//    If `f` was opened read-only, io61_flush(f) may either drop all
//    data buffered for reading, or do nothing.

int io61_flush(io61_file* f) {
    //(void) f;
    if (f->curr-f->start_in_file >0){
        write(f->fd, f->buffer, f->curr-f->start_in_file);
    }
    return 0;
}
// io61_seek(f, pos)
//    Change the file pointer for file `f` to `pos` bytes into the file.
//    Returns 0 on success and -1 on failure.
int io61_seek(io61_file* f, off_t pos) {
  
    if(f->mode ==O_WRONLY) {
        
        io61_flush(f);
        f->end_in_file = f->start_in_file;
    }
    off_t r = lseek(f->fd, (off_t) pos, SEEK_SET);
    if (r == (off_t) pos) {
        f->curr = r;
        return 0;
    } else {
        return -1;
    }
}
// io61_open_check(filename, mode)
//    Open the file corresponding to `filename` and return its io61_file.
//    If `filename == NULL`, returns either the standard input or the
//    standard output, depending on `mode`. Exits with an error message if
//    `filename != NULL` and the named file cannot be opened.

io61_file* io61_open_check(const char* filename, int mode) {
    int fd;
    if (filename) {
        fd = open(filename, mode, 0666);
    } else if ((mode & O_ACCMODE) == O_RDONLY) {
        fd = STDIN_FILENO;
    } else {
        fd = STDOUT_FILENO;
    }
    if (fd < 0) {
        fprintf(stderr, "%s: %s\n", filename, strerror(errno));
        exit(1);
    }
    return io61_fdopen(fd, mode & O_ACCMODE);
}

// io61_filesize(f)
//    Return the size of `f` in bytes. Returns -1 if `f` does not have a
//    well-defined size (for instance, if it is a pipe).

off_t io61_filesize(io61_file* f) {
    struct stat s;
    int r = fstat(f->fd, &s);
    if (r >= 0 && S_ISREG(s.st_mode)) {
        return s.st_size;
    } else {
        return -1;
    }
}


// io61_eof(f)
//    Test if readable file `f` is at end-of-file. Should only be called
//    immediately after a `read` call that returned 0 or -1.

int io61_eof(io61_file* f) {
    char x;
    ssize_t nread = read(f->fd, &x, 1);
    if (nread == 1) {
        fprintf(stderr, "Error: io61_eof called improperly\n\
  (Only call immediately after a read() that returned 0 or -1.)\n");
        abort();
    }
    return nread == 0;
}
