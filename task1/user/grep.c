#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char buf[1024];
int match(char *, char *, int);

char to_lower(char a)
{
    if (a >= 'A' && a <= 'Z')
        return a + 32;
    return a;
}

void grep(char *pattern, int fd, int ignore_case)
{
    int n, m;
    char *p, *q;

    m = 0;
    while ((n = read(fd, buf + m, sizeof(buf) - m - 1)) > 0)
    {
        m += n;
        buf[m] = '\0';
        p = buf;
        while ((q = strchr(p, '\n')) != 0)
        {
            *q = 0;
            if (match(pattern, p, ignore_case))
            {
                *q = '\n';
                write(1, p, q + 1 - p);
            }
            *q = '\n';
            p = q + 1;
        }
        if (p == buf)
            m = 0;
        if (m > 0)
        {
            m -= p - buf;
            memmove(buf, p, m);
        }
    }
}

int main(int argc, char *argv[])
{
    int fd, i;
    char *pattern;
    int ignore_case = 0;
    int start_index = 1;

    if (argc <= 1)
    {
        fprintf(2, "usage: grep [-i] pattern [file ...]\n");
        exit(1);
    }

    if (strcmp(argv[1], "-i") == 0)
    {
        ignore_case = 1;
        pattern = argv[2];
        start_index = 3;
    }
    else
    {
        pattern = argv[1];
        start_index = 2;
    }

    if (argc <= start_index)
    {
        grep(pattern, 0, ignore_case);
        exit(0);
    }

    for (i = start_index; i < argc; i++)
    {
        if ((fd = open(argv[i], 0)) < 0)
        {
            fprintf(2, "grep: cannot open %s\n", argv[i]);
            exit(1);
        }
        grep(pattern, fd, ignore_case);
        close(fd);
    }
    exit(0);
}

int matchhere(char *, char *, int);
int matchstar(int, char *, char *, int);

int match(char *re, char *text, int ignore_case)
{
    if (re[0] == '^')
        return matchhere(re + 1, text, ignore_case);
    do
    {
        if (matchhere(re, text, ignore_case))
            return 1;
    } while (*text++ != '\0');
    return 0;
}

int matchhere(char *re, char *text, int ignore_case)
{
    char t = *text;
    char r = *re;
    if (ignore_case)
    {
        t = to_lower(t);
        r = to_lower(r);
    }

    if (re[0] == '\0')
        return 1;
    if (re[1] == '*')
        return matchstar(re[0], re + 2, text, ignore_case);
    if (re[0] == '$' && re[1] == '\0')
        return *text == '\0';
    if (*text != '\0' && (re[0] == '.' || r == t))
        return matchhere(re + 1, text + 1, ignore_case);
    return 0;
}

int matchstar(int c, char *re, char *text, int ignore_case)
{
    char t = *text;
    char rc = c;
    if (ignore_case)
    {
        t = to_lower(t);
        rc = to_lower(rc);
    }

    do
    {
        if (matchhere(re, text, ignore_case))
            return 1;
        t = *text;
        if (ignore_case)
            t = to_lower(t);
    } while (*text != '\0' && (*text++ == c || c == '.'));
    
    return 0;
}