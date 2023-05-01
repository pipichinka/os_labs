#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/unistd.h>


int mymkdir(char* name){
    if (mkdir(name, 0777)){
        perror("mymkdir: removing dir");
        return 1;
    }
    return 0;
}


int myls(char* name){
    DIR* dir;
    if ((dir = opendir(name)) == NULL){
        perror("myls: openning dir");
        return 1;
    }
    struct dirent* ep;
    while ((ep = readdir (dir)) != NULL){
        printf("%s\n", ep->d_name);
    }    
    if (errno){
        perror("myls: reading dir");
        closedir(dir);
        return 1;
    }
    if (closedir(dir)){
        perror("myls: closing dir");
        return 1;
    }
    return 0;
}


int myrmdir(char* name){
    if (rmdir(name)){
        perror("myrmdir: removing dir");
        return 1;
    }
    return 0;
}


int mytouch(char* name){
    int fd = creat(name,0644);
    if (fd == -1){ 
        perror("mytouch: creating file");
        return 1;
    }
    if (close(fd)){
        perror("mytouch: closing created file");
    }
    return 0;
}


int mycat(char* name){
    FILE* file = fopen(name, "r");
    if (file == NULL){
        perror("mycat: openning file:");
    }
    size_t size;
    char buffer[4096];
    while((size = fread(buffer, sizeof(char), 4096, file))){
        if (write(0,buffer,size )== 0){
            perror("mycat: printing file");
            fclose(file);
            return 1;
        }
    }
    if (ferror(file)){
        perror("mycat: closing file");
        fclose(file);
        return 1;
    }
    if (fclose(file)){
        perror("mycat: closing file");
    }
    return 0;
}


int myrm(char* name){
    if (unlink(name)){
        perror("myrm: removing file");
        return 1;
    }
    return 0;
}


int myhl(char* name1, char* name2){
    if (link(name1, name2)){
        perror("myhl: linking files");
        return 1;
    }
    return 0;
}


int mylns(char* name1, char* name2){
    if (symlink(name1, name2)){
        perror("mylns: linking files");
        return 1;
    }
    return 0;
}


int mycatvsl(char* name){
    struct stat st;
    if (lstat(name, &st)){
        perror("mycatvsl: getting information about the link");
        return 1;
    }
    char buffer[st.st_size + 1];
    size_t len = readlink(name, buffer, st.st_size + 1);
    if (len == 0){
        perror("mycatvsl ");
        return 1;
    }
    buffer[len] = '\n';
    buffer[len +1] = '\0';
    if (write(1, buffer, len + 1) == 0){
        perror("mycatvsl ");
        return 1;
    }
    return 0;
}


int myinfo(char* name){
    struct stat st;
    if (stat(name, &st)){
        perror("myinfo: getting information about file");
        return 1;
    }

    printf("%s info: number of hard links: %lu.", name, st.st_nlink);
    char rights[10];
    unsigned long int rights_int = (__uintmax_t) st.st_mode;
    for (int i = 0; i < 3; i++){
        rights[3 * i ] = rights_int & (0400 >> (3 * i)) ? 'r':'-';
        rights[3 * i + 1] = rights_int & (0200 >> (3 * i)) ? 'w':'-';
        rights[3 * i + 2] = rights_int & (0100 >> (3 * i)) ? 'x':'-';
    }
    rights[9] ='\0';
    printf(" rights: %s\n", rights);
    return 0;
}


int mychmod(char* name, char* rights){
    struct stat st;
    if (stat(name, &st)){
        perror("mychmod: getting info about file");
        return 1;
    }
    st.st_mode = (st.st_mode >> 8) << 8;
    if (strlen(rights) != 9){
        fprintf(stderr, "invalid rights len(it should be 9)");
        return 1;
    }

    for (int i = 0; i < 9; i++){
        if (rights[i] != 'w' && rights[i] != 'r' && rights[i] != 'x' && rights[i] != '-'){
            fprintf(stderr, "invalid righs format(only letters r,w,x,- are allowed)");
            return 1;
        }
    }

    for (int i = 0; i < 3; i++){
        st.st_mode = rights[3 * i] == 'r' ? (st.st_mode | (0400 >> (3 * i))) : st.st_mode;
        st.st_mode = rights[3 * i + 1] == 'w' ? (st.st_mode | (0200 >> (3 * i))) : st.st_mode;
        st.st_mode = rights[3 * i + 2] == 'x' ? (st.st_mode | (0100 >> (3 * i))) : st.st_mode;
    }
    if (chmod(name, st.st_mode)){
        perror("mychmod: setting rights");
        return 1;
    }
    return 0;
}


int main(int argc, char** argv){
    if (argc == 0){
        printf("can't recognise hardlink at this program");
        return 1;
    }
    // if (argc == 1){
    //     printf("argv[0] %s\n", argv[0]);
    // }
    char* exe_name = strrchr(argv[0], '/') + 1;
    if (exe_name == (char* ) NULL + 1 ){
        fprintf(stderr,"error");
        return 1;
    }

    if (!strcmp(exe_name, "mymkdir")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: dir name)");
            return 1;
        }
        return mymkdir(argv[1]);
    }
    else if (!strcmp(exe_name, "myls")){
        if (argc == 1){
            return myls(".");
        }
        else if (argc > 2){
            fprintf(stderr, "invalid bymber of arguments(requars 1 argument: dir name. This prigram can be used with 0 arguments (\".\" used as argument))");
            return 1;
        }   
        return myls(argv[1]);
    }
    else if (!strcmp(exe_name, "myrmdir")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: dir name)");
            return 1;
        }
        return myrmdir(argv[1]);
    }
    else if (!strcmp(exe_name, "mytouch")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: file name)");
            return 1;
        }
        return mytouch(argv[1]);
    }
    else if (!strcmp(exe_name, "mycat")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: file name)");
            return 1;
        }
        return mycat(argv[1]);
    }
    else if (!strcmp(exe_name, "myrm")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: file name)");
            return 1;
        }
        return myrm(argv[1]);
    }
    else if (!strcmp(exe_name, "mylns")){
        if (argc != 3){
            fprintf(stderr, "ivalid number of arguments(requars 2 arg: original name, symlink name)");
            return 1;
        }
        return mylns(argv[1], argv[2]);
    }
    else if (!strcmp(exe_name, "mycatvsl")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: symlink name)");
            return 1;
        }
        return mycatvsl(argv[1]);
    }
    else if (!strcmp(exe_name, "mycatsl")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: symlink name)");
            return 1;
        }
        return mycat(argv[1]);
    }
    else if (!strcmp(exe_name, "myrmsl")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: symlink name)");
            return 1;
        }
        return myrm(argv[1]);
    }
    else if (!strcmp(exe_name, "myhl")){
        if (argc != 3){
            fprintf(stderr, "ivalid number of arguments(requars 2 arg: original name, new hardlink name)");
            return 1;
        }
        return myhl(argv[1], argv[2]);
    }
    else if (!strcmp(exe_name, "myrmhl")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: hardlink name)");
            return 1;
        }
        return myrm(argv[1]);
    }
    else if (!strcmp(exe_name, "myinfo")){
        if (argc != 2){
            fprintf(stderr, "ivalid number of arguments(requars 1 arg: file or dir name)");
            return 1;
        }
        return myinfo(argv[1]);
    }
    else if (!strcmp(exe_name, "mychmod")){
        if (argc != 3){
            fprintf(stderr, "ivalid number of arguments(requars 2 arg: file or dir name, rightsline (symbols w,r,x,-))");
            return 1;
        }
        return mychmod(argv[1], argv[2]);
    }
    

    fprintf(stderr, "can't recognise behave by this hardlink");
    return 1;


}
