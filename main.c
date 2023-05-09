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
#include "directory.h"
typedef struct dirent dirent;
#define BUFFER_LEN 4096
#define ERROR 1

long minl(long a, long b){
    if (a < b){
        return a;
    }
    return b;
}



int reverse_write_filein_to_fileout(FILE* file_in, FILE* file_out){
    if (file_in == NULL || file_out == NULL){
        return ERROR;
    }
    char buffer[BUFFER_LEN];
    if (fseek(file_in, 0, SEEK_END) == -1){
        perror("reverse_write_filein_to_fileout: fseek in filein");
        return ERROR;
    }
    long int file_in_len = ftell(file_in);
    if (file_in_len == -1){
        perror("reverse_write_filein_to_fileout: ftell in filein");
        return ERROR;
    }
    long int current_position = file_in_len;
    while (current_position != 0){
        long move = minl(current_position, BUFFER_LEN);
        if (fseek(file_in, (current_position - move), SEEK_SET) == -1){
            perror("reverse_write_filein_to_fileout: fseek in filein");
            return ERROR;
        }
        size_t read_size = fread(buffer, sizeof(char), (size_t) move, file_in);
        if (ferror(file_in)){
            perror("reverse_write_filein_to_fileout: reading filein");
            return ERROR;
        }
        for (size_t i = 0; i < read_size / 2; i++){
            char tmp = buffer[i];
            buffer[i] = buffer[read_size - i - 1];
            buffer[read_size - i - 1] = tmp;
        }
        fwrite(buffer, sizeof(char), read_size, file_out);
        if (ferror(file_in)){
            perror("reverse_write_filein_to_fileout: writing in fileout");
            return ERROR;
        }
        current_position -= move;
        if (fseek(file_in, current_position, SEEK_SET) == -1){
            perror("reverse_write_filein_to_fileout: fseek in filein");
            return ERROR;
        }
    }

    return 0;
} 


int reverse_string(char* string, int len, char* result){
    if (string == NULL || len < 0 || result == NULL){
        return ERROR;
    }
    for (int i = 0; i < len; i++){
        result[i] = string[len - i -1];
    }
    result[len] = '\0';
    return 0;
}


int reverse_inside_dir(Directory* in_directory, Directory* out_dirrectory){
    if (in_directory == NULL || out_dirrectory == NULL){
        return ERROR;
    }
    dirent *entry_element;
    DIR* dir = get_DIR(in_directory);
    if (dir == NULL){
        return ERROR;
    }
    while ((entry_element = readdir (dir)) != NULL){
        if (strcmp(entry_element->d_name, ".") == 0 || strcmp(entry_element->d_name, "..") == 0){
            continue;
        }
        char current_name[get_dir_name_size(in_directory) + strlen(entry_element->d_name) + 2];
        make_path(get_dir_name(in_directory), get_dir_name_size(in_directory), entry_element->d_name, strlen(entry_element->d_name), current_name);
        if (current_name == NULL){
            fprintf(stderr,"reverse_inside_dir: %s\n",strerror(errno));
            return ERROR;
        }
        printf("i: %s\n", current_name);
        char reverst_name[strlen(entry_element->d_name) + 1];
        reverse_string(entry_element->d_name, strlen(entry_element->d_name), reverst_name);
        char file_out_name[get_dir_name_size(out_dirrectory) + strlen(reverst_name) + 2];
        make_path(get_dir_name(out_dirrectory), get_dir_name_size(out_dirrectory), reverst_name, strlen(reverst_name), file_out_name);
        printf("o: %s\n", file_out_name);
        struct stat statbuf;
        if (stat(current_name, &statbuf)){
            fprintf(stderr, "reverse_inside_dir: stat at %s: %s.\n", current_name, strerror(errno));
            return ERROR;
        }

        if (S_ISREG(statbuf.st_mode)){
            FILE* file_in = fopen(current_name,"rb");
            if (file_in == NULL){
                fprintf(stderr, "reverse_inside_dir: openning file %s: %s", current_name, strerror(errno));
                return ERROR;
            }
            FILE* file_out = fopen(file_out_name, "wb");
            if (file_out == NULL){
                fprintf(stderr, "reverse_inside_dir: openning file %s: %s", file_out_name, strerror(errno));
                return ERROR;
            }
            if (chmod(file_out_name, statbuf.st_mode) == -1){
                fprintf(stderr, "reverse_inside_dir: chmod %s: %s", file_out_name, strerror(errno));
                return ERROR;
            }
            int error = reverse_write_filein_to_fileout(file_in, file_out);
            if (fclose(file_in)){
                fprintf(stderr, "reverse_inside_dir: closing %s: %s", current_name, strerror(errno));
                error = ERROR;
            }
            if (fclose(file_out)){
                fprintf(stderr, "reverse_inside_dir: closing %s: %s", file_out_name, strerror(errno));
                error = ERROR;
            }
            if (error == ERROR){
                return ERROR;
            }
        }
        else if (S_ISDIR(statbuf.st_mode)){
            Directory* next_in_dir = new_Directory(current_name,strlen(current_name));
            if (next_in_dir == NULL){
                return ERROR;
            }
            if (mkdir(file_out_name, statbuf.st_mode)){
                fprintf(stderr,"reverse_inside_dir: making dir %s: %s", file_out_name, strerror(errno));
                free_Directory(next_in_dir);
                return ERROR;
            }
            Directory* next_out_dir = new_Directory(file_out_name, strlen(file_out_name));
            if (next_out_dir == NULL){
                free_Directory(next_in_dir);
                return ERROR;
            }
            int error = reverse_inside_dir(next_in_dir, next_out_dir);
            free_Directory(next_in_dir);
            free_Directory(next_out_dir);
            if (error){
                return ERROR;
            }
        }
    }
    return 0;
}


int main(int argc, char** argv){
    if (argc != 2){
        fprintf(stderr,"no dir name given\n");
        return ERROR;    
    }
    struct stat statbuf;
    if (stat(argv[1], &statbuf)){
        fprintf(stderr,"main: getting information about %s: %s", argv[1], strerror(errno)); 
        return ERROR;
    }

    Directory* in_dir = new_Directory(argv[1], strlen(argv[1]));
    if (in_dir == NULL){
        return ERROR;
    }
    char reverst_name[strlen(argv[1]) + 1]; 
    reverse_string(argv[1], strlen(argv[1]), reverst_name);
    if (mkdir(reverst_name, statbuf.st_mode)){
        fprintf(stderr,"main: making dir %s: %s", reverst_name, strerror(errno)); 
        free_Directory(in_dir);
        return 1;
    }
    Directory* out_dir = new_Directory(reverst_name, strlen(reverst_name));
    if (out_dir == NULL){
        free_Directory(in_dir);
        return ERROR;
    }

    int error = reverse_inside_dir(in_dir, out_dir);
    free_Directory(in_dir);
    free_Directory(out_dir);
    if (error){
        return ERROR;
    }
    
    return 0;    
}
