#ifndef UTILS_H
#define UTILS_H
#include <sys/stat.h>

#define INCR 100

typedef struct fdtls {
    char * file_name;
    struct stat fs;
}  fdtls;

typedef struct fdtl_l {
    fdtls ** file_list;
    int current;
    int size;
} fdtl_l;

fdtls * create_file(char * file_name);
fdtl_l * create_file_list(void);
void append_file(fdtl_l * fl, fdtls * file);
void destroy_fdtl_l(fdtl_l * file_list);

#endif
