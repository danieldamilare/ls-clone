#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

fdtl_l * create_file_list(){
    fdtl_l*  file_list = malloc(sizeof(fdtl_l));
    if(file_list == NULL){
        fprintf(stderr, "Unable to allocate Memory\n");
        exit(EXIT_FAILURE);
    }
    file_list->file_list = malloc(INCR * sizeof(fdtls *));
    file_list->current = 0;
    file_list->size = INCR;
    return file_list;
}

fdtls * create_file(char * file_name){
    fdtls * file = malloc(sizeof(fdtls));
    file->file_name = strdup(file_name); 
    struct stat fs;
    int r = stat(file_name, &fs);
    if(r == -1){
        free(file->file_name);
        free(file);
        return NULL;
    }
    file->fs = fs;
    return file;
}

void append_file(fdtl_l *fl, fdtls * file){
    if(fl->current == fl->size){
        fdtls ** new_list = realloc(fl->file_list, (fl->size + INCR)* sizeof(fdtls *));
        if(new_list == NULL){
            fprintf(stderr, "Unable to reallocate memory\n");
            return;
        } 
        fl->file_list = new_list;
        fl->size += INCR;
    }
        fl->file_list[fl->current++] = file;
}

static void destroy_fdtls(fdtls * file){
    free(file->file_name);
    free(file);
}

void destroy_fdtl_l(fdtl_l * file_list){
    for(int i  = 0; i < file_list->current; i++){
        destroy_fdtls(file_list->file_list[i]);
    }
    free(file_list);
}
