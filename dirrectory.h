#include <dirent.h>

typedef struct Dirrectory Dirrectory;


Dirrectory* new_Dirrectory(char* dir_name, int dir_name_size);


void free_Dirrectory(Dirrectory* dir);


void open_Dirrectory(Dirrectory* dir);


DIR* get_DIR(Dirrectory* dir);


char* make_path(char* dir, int dir_name_size, char* file_name, int file_name_size);


char* get_dir_name(Dirrectory* dir);


int get_dir_name_size(Dirrectory* dir);