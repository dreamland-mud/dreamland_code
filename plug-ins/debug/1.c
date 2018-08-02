
#include <sys/syscall.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dlfcn.h>

int open(const char *path, int flags, ...);
int write(int fd, const char *b, int len);
int close(int d);
void exit(int rc);

extern int errno;


off_t __syscall(quad_t number, ...);

void *
mmap(addr, len, prot, flags, fd, offset)
        void *  addr;
        size_t  len;
        int     prot;
        int     flags;
        int     fd;
        off_t   offset;
{
    return ((void *)(intptr_t)__syscall((quad_t)SYS_mmap, addr, len, prot, flags, fd, 0, offset));
}

int 
strlen(const char *str)
{
    int i;
    
    for(i=0;str[i];i++)
	;
    
    return i;
}

int
puts(const char *str)
{
    write(1, str, strlen(str));
}

void
putc(char c)
{
    write(1, &c, 1);
}

void
map(const char *path, uintptr_t off, uintptr_t base, size_t sz, int wr)
{
    int fd = open(path, wr ? O_RDWR : O_RDONLY);
    void *p = mmap((void*)base, sz, (wr ? PROT_WRITE : 0 ) | PROT_READ | PROT_EXEC, MAP_SHARED | MAP_FIXED, fd, off);
    
    close(fd);
    if(p != (void*)base) {
	puts(path);
	puts(" :(\n");
	*(int*)0 = 0;
    }
}

void
map_all()
{
#include "patch.c"
}

int
main()
{
    map_all();

    (*(void (*)())0x54851f5c)(); // rtld: <lockdflt_init
    (*(void (*)(void *))0x5484e09c)((void*)0x54874000); // rtld: <allocate_initial_tls
    *(int*)0x54c84a88=0; // pthread: -- _thr_initial
    *(int*)0x54d6df84=0; // libc: __isthreaded
    *(int*)0x54c847f4=0; // pthread: thr_kern.c: inited
    
    //(*(void (*)(void *))0x54e852f8)((void*)0xae95b00); // save_room_objects
    (*(void (*)(void *, const char *))0x54e9ee18)((void*)0xd2a5800, "save"); // interpret
    //(*(void *(*)(const char *, int mode))0x548511d8)("../share/plugins/libdebug.so", RTLD_NOW); // dlopen
    puts(":)\n");
    exit(0);
}


