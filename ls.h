#ifndef LS_H
#define LS_H

#include "utils.h"
#define SIZE(arr) sizeof((arr)) /sizeof((arr)[0])
#define PATH_SEPERATOR '/'

#define KILO_SIZE 1000
#define DIR_COLOR   "\x1b[34m"
#define SYM_COLOR   "\x1b[36m" 
#define EXEC_COLOR  "\x1b[92m"
#define IMG_COLOR   "\x1b[35m"
#define ARC_COLOR   "\x1b[35m"
#define SOCK_COLOR  "\x1b[35m"
#define AUD_COLOR   "\x1b[36m"
#define BOLD        "\x1b[1m"
#define RESET       "\x1b[0m"
#

enum OptionFlags {
    IFLAG = 1 << 0,
    AFLAG = 1 << 1,
    RFLAG = 1 << 2,
    TFLAG = 1 << 3,
    HFLAG = 1 << 4,
    REFLAG = 1 << 5,
    SFLAG = 1 << 6,
    LFLAG = 1 << 7,
    DFLAG = 1 << 8,
};

extern int FLAGS;
void list_file(int no_of_files, char * files[no_of_files]);
void option_err1(char * option);
void option_err2(char option);
void print_perm_mode(unsigned);
void print_owner(unsigned);
void print_group(unsigned);
void sort_files(fdtl_l * fl, int flag);
void get_sizes(fdtls ** file, int total);
void  human_readable(unsigned long);
void ls(fdtls * file);
void ls_dir(fdtls * file);
void ls_l(fdtls * file);
char * get_base(char *filename);

int  is_archive(char *);
int is_media(char * );
int is_img(char *);
#endif
