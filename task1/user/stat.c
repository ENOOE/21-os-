#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char *type_to_string(int type)
{
    switch (type)
    {
    case T_DIR:
        return "DIR";
    case T_FILE:
        return "FILE";
    case T_DEVICE:
        return "DEVICE";
    case T_SYMLINK:
        return "SYMLINK";
    default:
        return "UNKNOWN";
    }
}

void stat_file(char *path, int no_follow)
{
    int fd;
    struct stat st;
    int mode = O_RDONLY;

    if (no_follow)
    {
        mode |= O_NOFOLLOW;
    }

    if ((fd = open(path, mode)) < 0)
    {
        fprintf(2, "stat: cannot stat %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "stat: cannot stat %s\n", path);
        close(fd);
        return;
    }

    printf("File:   %s\n", path);
    printf("Type:   %s\n", type_to_string(st.type));
    printf("Inode:  %d\n", st.ino);
    printf("Links:  %d\n", st.nlink);
    printf("Size:   %ld bytes\n", st.size);

    close(fd);
}

int main(int argc, char *argv[])
{
    int i;
    int no_follow = 0;
    int start_index = 1;

    if (argc < 2)
    {
        fprintf(2, "usage: stat [-L] file...\n");
        exit(1);
    }

    if (strcmp(argv[1], "-L") == 0)
    {
        no_follow = 1;
        start_index = 2;
    }

    for (i = start_index; i < argc; i++)
    {
        stat_file(argv[i], no_follow);
    }

    exit(0);
}