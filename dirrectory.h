#include <dirent.h>
#define ERROR_DEIRECTORY -1


typedef struct Directory Directory;

Directory* new_Directory(char* dir_name, int dir_name_size);

void free_Directory(Directory* dir);


void open_Directory(Directory* dir);


DIR* get_DIR(Directory* dir);


int make_path(char* dir, int dir_name_size, char* file_name, int file_name_size, char* result);


char* get_dir_name(Directory* dir);


int get_dir_name_size(Directory* dir);
