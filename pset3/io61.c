#include "io61.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>


// io61.c
//    YOUR CODE HERE!


// io61_file
//    Data structure for io61 file wrappers. Add your own stuff.
struct write_position {
    int activated;
    int start_for_write;
    int sz;
};
struct write_position w;
//w.activated = 0;
//w.start_for_write = 0;
//w.sz = 0;
struct io61_file {
    int fd;
    char buffer[4096];
    int fixed_start;
    int start;
    int end;
    int mode;
    int seek;
    int reverse;
    int start_in_file;
    int end_in_file;
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
    f->seek =0;
    f->end = 4096;
    f->reverse =4095;
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


// io61_readc(f)
//    Read a single (unsigned) character from `f` and return it. Returns EOF
//    (which is -1) on error or end-of-file.

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



int io61_readc(io61_file* f) {
    unsigned char buf[1];
    int m;
    if (f->seek ==1){
        
        
        
        if (f->reverse<4095){
            
            buf[0] = f->buffer[f->reverse];
            
            f->reverse = f->reverse - 1 ;
            
            if (f->reverse<0){
                
                f->reverse = 4095;
                
            }
            
            return buf[0];
            
        }
        
        else{
            
            int seek = lseek(f->fd,-4095,SEEK_CUR);
            
            if (seek>=0){
                
                int m = read(f->fd,f->buffer,4096);
                
                buf[0] = f->buffer[4095];
                
                f->reverse = 4094;
                
                return buf[0];
                
            }
            
            else{
                
                
                
                int y = read(f->fd,buf,1);
                
                int s = lseek(f->fd,-1,SEEK_CUR);
                
                if (y>0){
                    
                    return buf[0];
                    
                }
                
                else{
                    
                    return EOF;
                    
                }
                
            }
            
            
            
        }
        
    }
    
    
    





    if (f->start != 0){
        buf[0] = f->buffer[f->start];
        f->start = f->start + 1;
        if (f->start == f->end){
            f->start = 0;
        }
        return buf[0];
    }
    else{
        
        int m =read(f->fd, f->buffer, 4096);
        if (m > 0) {
            if (m!=4096) {
                f->end  = m;
                buf[0] = f->buffer[f->start];
                f->start = f->start + 1;
                
                if (f->start == f->end){
                    f->start = 0;
                }
                return buf[0];
                
            }
            else {
                buf[0] = f->buffer[f->start];
                f->start = f->start + 1;
                if (f->start == f->end){
                    f->start = 0;
                 }
                return buf[0];
            }
        }
        else {
            return EOF;
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
    
    size_t nread = 0;
    int m;
    if (f->seek !=10){
        w.activated = 1;
        w.start_for_write = f->curr;
        if (f->curr >= f->start_in_file && f->curr< f->end_in_file){
            if (f->curr+sz > f->end_in_file){
                //int y = lseek(f->fd,f->end_in_file, SEEK_SET);
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
                    w.sz = sz;
                    return remaining+left_over;
                }
                else if(m>0 && m<left_over) {
                    memcpy(buf+f->end_in_file-f->curr,f->buffer,m);
                    f->start_in_file = f->end_in_file;
                    f->end_in_file = f->end_in_file+m;
                    
                    f->curr = f->start_in_file+m;
                    w.sz = m;
                    return remaining+m;
                }
                else if (m==EOF || m<0){
                    w.sz = -1;
                    f->curr = f->end_in_file;
                    return remaining;
                }
                
            }
            else{
                memcpy(buf,f->buffer+f->curr-f->start_in_file,sz);
                f->curr = f->curr + sz;
                w.sz = sz;
                return sz;
            }
        }
        else{
            //int y = lseek(f->fd,f->curr, SEEK_SET);
            
            m = read_to_file(f);
            if (m>0 && m>=sz){
                memcpy(buf,f->buffer,sz);
                f->start_in_file = f->curr;
                f->end_in_file = f->curr+m;
                f->curr = f->curr+sz;
                w.sz = sz;
                return sz;
            }
            else if(m>0 && m<sz) {
                memcpy(buf,f->buffer,m);
                f->start_in_file = f->curr;
                f->end_in_file = f->curr+m;
                f->curr = f->curr+m;
                w.sz = m;
                return m;
            }
            else if (m<0 || m==EOF){
                return -1;
            }

        }
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    //size_t nread = 0;
    //int m;
    while (nread<sz){
        if (f->start ==0){
             m = read_to_file(f);
        }
        else{
             m = sz;
        }
        if ( m>0 && f->start<f->end){
            if (f->start+sz <= f->end){
                memcpy(buf, f->buffer+f->start,sz);
                f->start = f->start + sz;
                if (f->start==f->end){
                    f->start = 0;
                }
                if (f->start>f->end){
                    f->start = 0;
                //lseek(f,f->start-f->end,SEEK_CUR);
                }
                nread= nread+sz-nread;
                return nread;
            }
            else{
                memcpy(buf,f->buffer+f->start,f->end-f->start);
                int left_over  = f->start+sz-f->end;
                int temp = f->start;
                f->start = 0;
                int x = read_to_file(f);
                if (x > 0 && x>=left_over){
                    memcpy(buf+f->end-temp,f->buffer,left_over);
                    f->start = f->start + left_over;
                    f->end = x;
                    return f->end-temp+left_over;
                }
                else if (x>0 && x<left_over){
                    memcpy(buf+f->end-temp,f->buffer,x);
                    f->start = f->start + x;
                    f->end = x;
                    return f->end-temp+x;
                }
                
                //int x = read(f->fd,buf+f->end-f->start,f->start+sz-f->end);
                else if (x<0 || x==EOF){
                    return f->end-temp;
                }
                
                
            }
        }
        
        else if (m>0 && m==sz){
            memcpy(buf, f->buffer,sz-nread);
            f->start = 0;
            nread= nread+f->end;
            return nread;
            
            
        }
        else if(m==EOF){
            return EOF;
            
        }
        else {
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
    //size_t nwritten = 0;
    //while (nwritten != sz) {
        //if (io61_writec(f, buf[nwritten]) == -1) {
            //break;
        //}
        //++nwritten;
    //}
    //if (nwritten != 0 || sz == 0) {
        //return nwritten;
    //} else {
        //return -1;
    //}
    //my code
    if (w.activated==100){
        //int y = lseek(f->fd,w.start_for_write, SEEK_SET);
        int x = write(f->fd,buf,w.sz);
        return x;
    }
    
    int x = write(f->fd,buf,sz);
    return x;
    
    
    if(f->start<f->end && f->end-f->start>=sz){
        memcpy(f->buffer+f->start,buf,sz);
        f->start = f->start + sz;
        return sz;
        
    }
    else if( f->start<f->end && f->end-f->start<sz){
        memcpy(f->buffer+f->start,buf,f->end-f->start);
        int temp = f->start;
        int left_over = f->start+sz-f->end;
        int x = write(f->fd,f->buffer,4096);
        f->start  = 0;
        memcpy(f->buffer+f->start,buf+f->end-temp,left_over);
        f->start = f->start + left_over;
    }
    else{
        f->start = 0;
        write(f->fd,f->buffer,4096);
        memcpy(f->buffer+f->start,buf,sz);
        f->start = f->start + sz;
    }
    
}


// io61_flush(f)
//    Forces a write of all buffered data written to `f`.
//    If `f` was opened read-only, io61_flush(f) may either drop all
//    data buffered for reading, or do nothing.

int io61_flush(io61_file* f) {
    (void) f;
    return 0;
}


// io61_seek(f, pos)
//    Change the file pointer for file `f` to `pos` bytes into the file.
//    Returns 0 on success and -1 on failure.

int io61_seek(io61_file* f, off_t pos) {
    f->seek = 1;
    
    off_t r = lseek(f->fd, (off_t) pos, SEEK_SET);
    if (r == (off_t) pos) {
        f->curr = r;
        return 0;
    } else {
        return -1;
    }
}


// You shouldn't need to change these functions.

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
