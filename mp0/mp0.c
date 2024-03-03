#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"
#include "kernel/memlayout.h"
#include "kernel/riscv.h"

struct counter
{
    int dir;
    int file;
};

int keycnt(char* path, char key);
void traversefile(char* path, struct dirent entry, char key, struct counter* cnt);
void traversedir(char* path, char key, struct counter* cnt);

int keycnt(char* path, char key)
{
    int ans = 0;
    for(int i = 0; i < strlen(path); i++)
    {
        if(path[i] == key)
            ans += 1;
    }
    return ans;
}

void traversefile(char* path, struct dirent entry, char key, struct counter* cnt)
{
    char pathname[512] = {};
    memcpy(pathname, path, strlen(path)+1);
    pathname[strlen(path)] = '/';
    memcpy(pathname+strlen(path)+1, entry.name, strlen(entry.name)+1);
    //int len = snprintf(pathname, sizeof(pathname) - 1, "%s/%s", path, entry->d_name);
    //pathname[len] = '\0';

    struct stat st = {};
    stat(pathname, &st);
    if(st.type == T_DIR)
        traversedir(pathname, key, cnt);
    else if(st.type == T_FILE)
    {
        cnt->file += 1;
        printf("%s %d\n", pathname, keycnt(pathname, key));
        //printf("----> %d dir, %d file\n", cnt->dir, cnt->file);
    }
    return;
}

void traversedir(char* path, char key, struct counter* cnt)
{
    int fd;
    struct stat st;
    if ((fd = open(path, 0)) < 0){
        //printf("cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        //printf("cannot stat %s\n", path);
        return;
    } else {
        close(fd);
    }

    if(cnt->dir == 0 && st.type == T_FILE)
    {
        cnt->file += 1;
        return;
    }
        
    cnt->dir += 1;
    printf("%s %d\n", path, keycnt(path, key));
    //printf("----> %d dir, %d file\n", cnt->dir, cnt->file);
    struct dirent en;
    if ((fd = open(path, 0)) < 0){
        //printf("cannot stat %s\n", path);
        return;
    }

    while(read(fd, &en, sizeof(en)) == sizeof(en))
    {
        if (strcmp(en.name, ".") == 0 || strcmp(en.name, "..") == 0)
            continue;
        traversefile(path, en, key, cnt);
    }
    close(fd);
    return;
}

int main(int argc, char* argv[])
{
    int pid;
    int fd[2];

    //check arguments
    if (argc < 3) {
        //printf("missing arguments\n");
        exit(-1);
    }

    //copy the root dir and key
    char* path = argv[1];
    char key = *argv[2];

    //create pipe //0 read//1 write
    if (pipe(fd) < 0) {
        //printf("pipe failed\n");
        exit(-1);
    }

    pid =  fork();
    if(pid == 0) //child
    {
        struct counter cnt;
        cnt.dir = 0;
        cnt.file = 0;
        close(fd[0]);
        traversedir(path, key, &cnt);
        write(fd[1], &cnt, sizeof(struct counter));
        close(fd[1]);
    }
    else if(pid > 0) //parent
    {
        struct counter cnt;
        close(fd[1]);
        wait(0);
        read(fd[0], &cnt, sizeof(struct counter));
        if(cnt.dir == 0)
            printf("%s [error opening dir]\n", path);
        else
            cnt.dir -= 1; //minus the root directory 
        printf("\n%d directories, %d files\n", cnt.dir, cnt.file);
        close(fd[0]);
    }
    else
    {
        //printf("fork failed\n");
        exit(-1);
    }

    exit(0);
}