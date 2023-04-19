#include <stdio.h>
#include <malloc.h>
#include <dirent.h>
#include <stdint.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <string.h>
#include "dirrectory.h"
typedef struct dirent dirent;
#define BUFFER_LEN 4096


long minl(long a, long b){
    if (a < b){
        return a;
    }
    return b;
}



int reverse_write_filein_to_fileout(FILE* file_in, FILE* file_out){
    if (file_in == NULL || file_out == NULL){
        return 1;
    }
    char buffer[BUFFER_LEN];
    if (fseek(file_in, 0, SEEK_END) == -1){
        perror("error during reading file0 ");
        return 1;
    }
    long int file_in_len = ftell(file_in);
    if (file_in_len == -1){
        perror("error during reading file2 ");
        return 1;
    }
    long int current_position = file_in_len;
    while (current_position != 0){
        long move = minl(current_position, BUFFER_LEN);
        if (fseek(file_in, (current_position - move), SEEK_SET) == -1){
            perror("error during reading file1 ");
            return 1;
        }
        size_t read_size = fread(buffer, sizeof(char), (size_t) move, file_in);
        if (ferror(file_in)){
            perror("error during reading file3 ");
            return 1;
        }
        for (size_t i = 0; i < read_size / 2; i++){
            char tmp = buffer[i];
            buffer[i] = buffer[read_size - i - 1];
            buffer[read_size - i - 1] = tmp;
        }
        fwrite(buffer, sizeof(char), read_size, file_out);
        if (ferror(file_in)){
            perror("error during writing file ");
            return 1;
        }
        current_position -= move;
        if (fseek(file_in, current_position, SEEK_SET) == -1){
            perror("error during reading file 4");
            return 1;
        }
    }

    return 0;
} 


char* reverse_string(char* string, int len){
    if (string == NULL || len < 0){
        return NULL;
    }
    char* result = (char*) malloc(sizeof(char) * (len + 1));
    if (result == NULL){
        return NULL;
    }
    for (int i = 0; i < len; i++){
        result[i] = string[len - i -1];
    }
    result[len] = '\0';
    return result;
}


int reverse_inside_dir(Dirrectory* in_directory, Dirrectory* out_dirrectory){
    if (in_directory == NULL || out_dirrectory == NULL){
        return 1;
    }
    dirent *ep;
    DIR* dir = get_DIR(in_directory);
    if (dir == NULL){
        return 1;
    }
    while ((ep = readdir (dir)) != NULL){
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0){
            continue;
        }
        char* current_name = make_path(get_dir_name(in_directory), get_dir_name_size(in_directory), ep->d_name, strlen(ep->d_name));
        if (current_name == NULL){
            fprintf(stderr,"reverse_inside_dir: %s",strerror(errno));
            return 1;
        }
        printf("i: %s\n", current_name);
        char* reverst_name = reverse_string(ep->d_name, strlen(ep->d_name));
        char* file_out_name = make_path(get_dir_name(out_dirrectory), get_dir_name_size(out_dirrectory), reverst_name, strlen(reverst_name));
        free(reverst_name);
        printf("o: %s\n", file_out_name);
        if (file_out_name == NULL){
            free(reverst_name);
            free(current_name);
            printf("reverse_inside_dir: %s",strerror(errno));
            return 1;
        }
        struct stat statbuf;
        stat(current_name, &statbuf);


        if (S_ISREG(statbuf.st_mode)){
            FILE* file_in = fopen(current_name,"rb");
            if (file_in == NULL){
                free(current_name);
                free(file_out_name);
                printf("reverse_inside_dir: openning file %s: %s", current_name, strerror(errno));
                return 1;
            }
            FILE* file_out = fopen(file_out_name, "wb");
            if (file_out == NULL){
                printf("reverse_inside_dir: openning file %s: %s", file_out_name, strerror(errno));
                free(file_out_name);
                free(current_name);
                return 1;
            }
            if (chmod(file_out_name, statbuf.st_mode) == -1){
                printf("reverse_inside_dir: chmod %s: %s", file_out_name, strerror(errno));
                free(file_out_name);
                free(current_name);
                return 1;
            }
            int error = reverse_write_filein_to_fileout(file_in, file_out);
            fclose(file_in);
            fclose(file_out);
            if (error){
                free(file_out_name);
                free(current_name);
                return 1;
            }
        }
        else if (S_ISDIR(statbuf.st_mode)){
            Dirrectory* next_in_dir = new_Dirrectory(current_name,strlen(current_name));
            if (next_in_dir == NULL){
                free(current_name);
                free(file_out_name);
                return 1;
            }
            if (mkdir(file_out_name, statbuf.st_mode)){
                printf("reverse_inside_dir: making dir %s: %s", file_out_name, strerror(errno));
                free_Dirrectory(next_in_dir);
                free(current_name);
                free(file_out_name);
                return 1;
            }
            Dirrectory* next_out_dir = new_Dirrectory(file_out_name, strlen(file_out_name));
            if (next_out_dir == NULL){
                free(file_out_name);
                free_Dirrectory(next_in_dir);
                free(file_out_name);
                return 1;
            }
            int error = reverse_inside_dir(next_in_dir, next_out_dir);
            free_Dirrectory(next_in_dir);
            free_Dirrectory(next_out_dir);
            if (error){
                free(file_out_name);
                free(current_name);
                return 1;
            }
        }
        free(file_out_name);
        free(current_name);
    }
    return 0;
}


int main(int argc, char** argv){
    if (argc != 2){
        printf("no dir name give\n");
        return 1;    
    }
    struct stat statbuf;
    if (stat(argv[1], &statbuf)){
        printf("main: getting information about %s: %s", argv[1], strerror(errno)); 
        return 1;
    }

    Dirrectory* in_dir = new_Dirrectory(argv[1], strlen(argv[1]));
    if (in_dir == NULL){
        return 1;
    }
    char* reverst_name = reverse_string(argv[1], strlen(argv[1]));
    if (reverst_name == NULL){
        printf("main: %s", strerror(errno)); 
        free_Dirrectory(in_dir);
        return 1;
    }
    if (mkdir(reverst_name, statbuf.st_mode)){
        printf("main: making dir %s: %s", reverst_name, strerror(errno)); 
        free_Dirrectory(in_dir);
        free(reverst_name);
        return 1;
    }
    Dirrectory* out_dir = new_Dirrectory(reverst_name, strlen(reverst_name));
    if (out_dir == NULL){
        free(reverst_name);
        free_Dirrectory(in_dir);
        return 1;
    }

    int error = reverse_inside_dir(in_dir, out_dir);
    free(reverst_name);
    free_Dirrectory(in_dir);
    free_Dirrectory(out_dir);
    if (error){
        return 1;
    }
    
    return 0;    
}