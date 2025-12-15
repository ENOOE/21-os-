#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    if (argc != 3 && argc != 4)
    {
        fprintf(2, "usage: ln [-s] target newfile\n");
        exit(1);
    }

    if (strcmp(argv[1], "-s") == 0)
    {
        if (symlink(argv[2], argv[3]) < 0)
        {
            fprintf(2, "ln: failed to create symlink %s -> %s\n", argv[3], argv[2]);
            exit(1);
        }
    }
    else
    {
        if (link(argv[1], argv[2]) < 0)
        {
            fprintf(2, "ln: failed to link %s to %s\n", argv[1], argv[2]);
            exit(1);
        }
    }
    exit(0);
}