uint64 sys_symlink(void)
{
    char target[MAXPATH], path[MAXPATH];
    struct inode *ip;

    if (argstr(0, target, MAXPATH) < 0 || argstr(1, path, MAXPATH) < 0)
        return -1;

    begin_op();
    ip = create(path, T_SYMLINK, 0, 0);
    if (ip == 0)
    {
        end_op();
        return -1;
    }
    if (writei(ip, 0, (uint64)target, 0, MAXPATH) < MAXPATH)
    {
        iunlockput(ip);
        end_op();
        return -1;
    }
    iunlockput(ip);
    end_op();
    return 0;
}

uint64 sys_open(void)
{
    char path[MAXPATH];
    int fd, omode;
    struct file *f;
    struct inode *ip;
    int n;

    if (argstr(0, path, MAXPATH) < 0 || argint(1, &omode) < 0)
        return -1;

    begin_op();

    if (omode & O_CREATE)
    {
        ip = create(path, T_FILE, 0, 0);
        if (ip == 0)
        {
            end_op();
            return -1;
        }
    }
    else
    {
        if ((ip = namei(path)) == 0)
        {
            end_op();
            return -1;
        }
        ilock(ip);

        // === 软链接跟随逻辑 START ===
        if (ip->type == T_SYMLINK && !(omode & O_NOFOLLOW))
        {
            int depth = 0;
            char target[MAXPATH];
            while (ip->type == T_SYMLINK)
            {
                if (depth >= 10)
                { // 防止死循环
                    iunlockput(ip);
                    end_op();
                    return -1;
                }
                if (readi(ip, 0, (uint64)target, 0, MAXPATH) <= 0)
                {
                    iunlockput(ip);
                    end_op();
                    return -1;
                }
                iunlockput(ip);
                if ((ip = namei(target)) == 0)
                {
                    end_op();
                    return -1;
                }
                ilock(ip);
                depth++;
            }
        }
        // === 软链接跟随逻辑 END ===

        if (ip->type == T_DIR && omode != O_RDONLY)
        {
            iunlockput(ip);
            end_op();
            return -1;
        }
    }

    if ((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0)
    {
        if (f)
            fileclose(f);
        iunlockput(ip);
        end_op();
        return -1;
    }

    if (ip->type == T_DEVICE)
    {
        f->type = FD_DEVICE;
        f->major = ip->major;
    }
    else
    {
        f->type = FD_INODE;
        // === 重定向追加逻辑 START ===
        if (omode & O_APPEND)
            f->off = ip->size;
        else
            f->off = 0;
        // === 重定向追加逻辑 END ===
    }

    f->ip = ip;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

    if ((omode & O_TRUNC) && ip->type == T_FILE)
    {
        itrunc(ip);
    }

    iunlock(ip);
    end_op();

    return fd;
}