#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include "ls.h"

int FLAGS = 0X00;
static int tm_size;
static int rt_size;
static int grp_size;
static int  sz_size;

int main(int argc, char ** argv){
    char buf[BUFSIZ];
    setbuf(stdout, buf);
    char * files[argc];
    int file_counter = 0;
    char * options[] = {"inode", "all", "reverse",  "recursive", "human-readable", "size", "directory"};
    int option_flag[] = {IFLAG, AFLAG, RFLAG, REFLAG, HFLAG, SFLAG, DFLAG};

    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            int j = 1;
            if(argv[i][j] == '-'){
                int match = 0;
                int size = SIZE(options);
                for(int k = 0; k < size; k++){
                    if(strcmp(options[k], &argv[i][j+1]) == 0){
                        FLAGS |= option_flag[k];
                        match = 1;
                        break;
                    }
                }
                if(!match) option_err1(&argv[i][j+1]);
            }
            else {
                int len = strlen(argv[i]);
                while(j <len){
                    switch(argv[i][j]){
                        case 'i':
                            FLAGS |= IFLAG;
                            break;
                        case 'a':
                            FLAGS |= AFLAG;
                            break;
                        case 'r':
                            FLAGS |= RFLAG;
                            break;
                        case 'R':
                            FLAGS |= REFLAG;
                            break;
                        case 't':
                            FLAGS |= TFLAG;
                            break;
                        case 'h':
                            FLAGS |= HFLAG;
                            break;
                        case 's':
                            FLAGS |= SFLAG;
                            break;
                        case 'l':
                            FLAGS |= LFLAG;
                            break;
                        default:
                            option_err2(argv[i][j]);
                    }
                    j++;
                }
            }
        }
        else {
            files[file_counter++] = argv[i];
        }
    }

    if(file_counter == 0){
        files[file_counter++] = ".";
    }
    list_file(file_counter, files);
}

void option_err1(char * option){
    fprintf(stderr, "ls: unrecognized option -- %s", option);
    exit(1);
}

void option_err2(char option){
    fprintf(stderr, "ls: invalid option -- %c\n", option);
    exit(1);
}


char * get_base(char * filename){
    char * base = strrchr(filename, PATH_SEPERATOR);
    return(base? base+1 : filename);
}


void list_file(int no_files, char * files[no_files]){
    //create file_list;
    fdtl_l * fl;
    fl = create_file_list();
    if(!fl){
        fprintf(stderr, "Error...\n");
        return;
    }
    for(int i  = 0; i < no_files; i++){
        fdtls * file = create_file(files[i]);
        if(file == NULL){
            fprintf(stderr, "ls: cannot access '%s': No such file or directory\n", files[i]);
            continue;
        } 
        append_file(fl, file);
    }

    
    if(FLAGS & TFLAG) sort_files(fl, TFLAG);
    else if(FLAGS & SFLAG)sort_files(fl, SFLAG);
    else sort_files(fl, 0);

    int current = fl->current;
    get_sizes(fl->file_list, fl->current);
    for(int i = 0; i < current; i++){
        fdtls * file = fl->file_list[i];

        if(S_ISDIR(file->fs.st_mode)){

            ls_dir(file);
        }
        else{
            ls(file);
        }
         

    }
    if(!(FLAGS & LFLAG)) printf("\n");
    destroy_fdtl_l(fl);
 }

void ls(fdtls * file){
    if(FLAGS & LFLAG){
        ls_l(file);
    }
    else {

        if(FLAGS & IFLAG){
            printf("%lu  ", file->fs.st_ino);
        }
        printf("%s\t", get_base(file->file_name));
    }
}

void ls_l(fdtls * file){
    if(FLAGS & IFLAG){
        printf("%lu", file->fs.st_ino);
    }
    print_perm_mode(file->fs.st_mode);
    printf("%lu ", file->fs.st_nlink);
    print_owner(file->fs.st_uid);
    print_group(file->fs.st_gid);

    if(FLAGS & HFLAG){
        human_readable(file->fs.st_size);
    }
    else printf("%*lu ", sz_size,  file->fs.st_size);
    struct tm * time = localtime(&(file->fs.st_mtim));
    char time_text[100];
    strftime(time_text, 100, "%b %d %R", time);
    printf("%*s ",tm_size, time_text);
    printf("%s\n", get_base(file->file_name));
}

void ls_dir(fdtls * file){
    DIR * directory;
    struct dirent * dir;
    directory = opendir(file->file_name);
    if(directory == NULL){
        fprintf(stderr, "ls: cannot access '%s': No such file or directory", file->file_name);
        return;
    }
    fdtl_l * fl = create_file_list();
    if(!fl){
        fprintf(stderr, "Error...\n");
    }
    printf("\n\n'%s':\n", file->file_name);

    while((dir = readdir(directory)) != NULL){
        char word[BUFSIZ];
        if(strcmp(dir->d_name, ".") && strcmp(dir->d_name, ".."))
            sprintf(word, "%s/%s",file->file_name, dir->d_name);
        else
            continue;
        fdtls * new_file = create_file(word);
        if(!new_file){
            fprintf(stderr, "ls: cannot access '%s': No such file or directory", word);
            continue;
        }
        append_file(fl, new_file);
    }

    if(FLAGS & TFLAG) sort_files(fl, TFLAG);
    else if(FLAGS & SFLAG)sort_files(fl, SFLAG);
    else sort_files(fl, 0);

    int current = fl->current;
    get_sizes(fl->file_list, current);

    for(int i = 0; i < current; i++){
        fdtls * cur = fl->file_list[i];
        if (!(FLAGS && AFLAG))
            ;
        if(S_ISDIR(cur->fs.st_mode) && (FLAGS & REFLAG)){
            ls_dir(cur);
        }
        else{
            if(FLAGS & LFLAG) ls_l(cur);
            else ls(cur);
        }
    }
    closedir(directory);
    destroy_fdtl_l(fl);
}


static char * h_readable(unsigned long size){
    double d_size = size;
    char sizes[] = {'B', 'K', 'M', 'G', 'T'};
    int current_size = 0;
    while(d_size > KILO_SIZE){
        d_size /= KILO_SIZE;
        current_size++;
    }
    char return_str[5];
    if((d_size - (int)d_size) == 0){
        sprintf(return_str, "%d%c", (int)d_size, sizes[current_size]);
    } else if((d_size - (int)d_size) <= 0.1){
        sprintf(return_str, "%d.0%c", (int)d_size, sizes[current_size]);
    }

    else sprintf(return_str, "%.1lf%c", d_size, sizes[current_size]);
    return strdup(return_str);

}

void human_readable(unsigned long size){
    char *return_str = h_readable(size);
    printf("%-*s ", sz_size,  return_str);
    free(return_str);
}

void print_owner(unsigned uid){
    struct passwd * pwd = getpwuid(uid);
    printf("%-*s ", rt_size, pwd->pw_name);
}

void print_group(unsigned gid){
    struct group * grp = getgrgid(gid);
    printf("%-*s ",grp_size, grp->gr_name);
}


int compare_time(void * s1, void *s2){
    struct  timespec time1, time2;
    time1 = ((struct stat *)s1)->st_mtim;
    time2 = ((struct stat *)s2)->st_mtim;
    int diff = (time1.tv_sec -time2.tv_sec) + (time1.tv_nsec - time2.tv_nsec)/1e9;
    if(diff > 0){
        if (FLAGS & RFLAG){
            return 1;
        }
        else return -1;
    }
    else if(diff < 0){
        if (FLAGS & RFLAG) return -1;
        else return 1;
    }
    else return 0;
}

int compare_size(void * s1, void *s2){
    unsigned size1= ((struct stat *)s1)->st_size;
    unsigned size2 = ((struct stat *)s2)->st_size;
    int diff = (int)size1 - (int)size2;
    if(diff > 0){
        if (FLAGS & RFLAG)return 1;
        else return -1;
    }
    else if (diff < 0){
        if(FLAGS & RFLAG) return -1;
        else return 1;
    }
    else return 0;
}

void print_perm_mode(unsigned mode){
    char perm[11];
    char *p = perm;
    if(S_ISREG(mode))
       *p++ = '-';
    else if (S_ISDIR(mode))
        *p++='d';
    else if (S_ISBLK(mode)) *p++ = 'b';
    else if (S_ISCHR(mode)) *p++ = 'c'; 
    else if (S_ISLNK(mode)) *p++ = 'l';
    else if (S_ISFIFO(mode)) *p++ = 'p';
    else if (S_ISSOCK(mode)) *p++ = 's';

    *p++ = (mode & S_IRUSR)? 'r' : '-';
    *p++ = (mode & S_IWUSR)? 'w' : '-';
    *p++ = (mode & S_IXUSR)? 'x' : '-';
    *p++ = (mode & S_IRGRP)? 'r' : '-';
    *p++ = (mode & S_IWGRP)? 'w' : '-';
    *p++ = (mode & S_IXGRP)? 'x' : '-';
    *p++ = (mode & S_IROTH)? 'r' : '-';
    *p++ = (mode & S_IWOTH)? 'w' : '-';
    *p++ = (mode & S_IXOTH)? 'x' : '-';
    *p = '\0';
    printf("%s ", perm);
}

static int partition(fdtls ** files, int size){
    int files_no = 0;

    for(int i = 0; i < size; i++){
        if(S_ISREG(files[i]->fs.st_mode)){
            fdtls * temp = files[files_no];
            files[files_no++] = files[i];
            files[i] = temp;
        }
    }
    return files_no;
}

void sort_files(fdtl_l * fl, int flag) {
    int cmp_time(const void *, const void *);
    int cmp_size(const void *, const void *);
    int cmp_name(const void *, const void *);

    int part = partition(fl->file_list, fl->current);

    int (*comparator)(const void *, const  void *);

    if (flag == TFLAG)
        comparator = cmp_time;
    else if (flag == SFLAG)
        comparator = cmp_size;
    else 
        comparator = cmp_name;
    
    qsort(fl->file_list, part, sizeof(fdtls *), comparator); 
    qsort(fl->file_list+part, fl->current - part, sizeof(fdtls *), comparator);
}


int cmp_time(const void * f1, const void * f2){
    int result = compare_time(&(*(fdtls **)f1)->fs, &(*(fdtls **)f2)->fs);
    return result;
}

int cmp_size(const void * f1, const void * f2){
    int result = compare_size(&(*(fdtls **)f1)->fs, &(*(fdtls **)f2)->fs);

   return result;

}

int cmp_name(const void * f1, const void * f2){
    int result = strcmp((*(fdtls **) f1 )->file_name, (*(fdtls **) f2 )->file_name);
    if(FLAGS & RFLAG){ 
       return -result;
    }
    else {
       return result;
    }
}

void get_sizes(fdtls ** file, int total){
    for(int i = 0; i <total; i++){

        if(FLAGS &HFLAG){
            char * result = h_readable(file[i]->fs.st_size);
            if(strlen(result) > sz_size) sz_size = strlen(result);
            free(result);
        }
        else {
            int count = 0, n = file[i]->fs.st_size;
            while(n > 0) count++, n/= 10;
            if(count > sz_size) sz_size = count;
        }

    char * grpname =  getgrgid(file[i]->fs.st_gid)->gr_name;
    char * rtname= getpwuid(file[i]->fs.st_uid)->pw_name;
    if(strlen(rtname) > rt_size) rt_size = strlen(rtname);
    if(strlen(grpname) > grp_size) grp_size = strlen(grpname);

    char time_text[100];
    struct tm * time = localtime(&(file[i]->fs.st_ctime));
    strftime(time_text, 100, "%b %d %R", time);
    if(strlen(time_text) > tm_size) tm_size = strlen(time_text);
    }
}



void print_file(fdtls * file){
    struct stat file_st = file->fs;
    char * word;

    if (S_ISLNK(file_st.st_mode)){

        if(FLAGS & LFLAG){
            char word[256];

            if(readlink(file->file_name, word, 256) != -1){
                printf("%s%s%s%s-> %s%s%s%s", BOLD, SYM_COLOR, file->file_name, RESET, DIR_COLOR, BOLD, word, RESET);
            }
            else {
                printf("%s\x1b[40m\x1b[31m%s%s", BOLD, file->file_name, RESET);
            }
        }
        else {
            printf("%s%s%s%s\n", BOLD, SYM_COLOR, file->file_name, RESET);
        }
    }

    else if(S_ISDIR(file_st.st_mode))
        printf("%s%s%s", DIR_COLOR, file->file_name, RESET);

    else if((file_st.st_mode & S_IXGRP) || (file_st.st_mode & S_IXUSR) || (file_st.st_mode & S_IXOTH)){
       printf("%s%s%s%s", BOLD, EXEC_COLOR, file->file_name, RESET); 
    }
    
    else if(is_archive(file->file_name)){
        printf("%s%s%s%s", BOLD, ARC_COLOR, file->file_name, RESET);
    }
    else if(is_img(file->file_name)){
        printf("%s%s%s%s", BOLD, IMG_COLOR, file->file_name, RESET);
    }
    else if (is_media(file->file_name)){
        printf("%s%s%s%s", BOLD, AUD_COLOR, file->file_name, RESET);
    }

}
