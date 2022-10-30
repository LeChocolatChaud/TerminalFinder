#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

static int stringCompare(const void *a, const void *b)
{
    return strcasecmp(*(const char **)a, *(const char **)b);
}

int main(int argc, char **argv)
{
    // init path
    char *path;
    path = (char *)malloc(sizeof("/"));
    strcpy(path, "/");
    if (argc >= 2)
    {
        path = (char *)realloc(path, sizeof(argv[1]));
        strcpy(path, argv[1]);
    }

    while (1)
    {
        //dir validation
        if (strrchr(path, '/') != path + strlen(path) - 1)
        {
            path = (char *)realloc(path, sizeof(path) + sizeof('/'));
            strcat(path, "/");
        }

        // init dir
        DIR *curDir = opendir(path);

        if (curDir == NULL)
        {
            printf("tfin: No directory named %s is found\n", path);
            return 1;
        }
        else if (readdir(curDir)->d_type != DT_DIR)
        {
            printf("tfin: %s is not a directory.\n", path);
            return 2;
        }

        printf("\n");

        // init window size
        struct winsize wsize;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsize);
        wsize.ws_row--;

        // read entries
        struct dirent *curDirEntry;
        int dafsCount = 0;

        while ((curDirEntry = readdir(curDir)) != NULL)
        {
            dafsCount++;
        }

        char *dafs[dafsCount];
        int maxl;

        rewinddir(curDir);

        for (int i = 0; i < dafsCount; i++)
        {
            char *curEntryName = readdir(curDir)->d_name;
            for (int j = 0; j < strlen(curEntryName); j++)
            {
                if (
                    curEntryName[j] == 0x08 ||
                    curEntryName[j] == 0x0a ||
                    curEntryName[j] == 0x0d)
                {
                    curEntryName[j] = '?';
                }
            }
            *(dafs + i) = curEntryName;
            maxl = strlen(dafs[i]) > maxl ? strlen(dafs[i]) : maxl;
        }

        qsort(dafs, dafsCount, sizeof(const char *), stringCompare);

        // print entries

        /* for debug (4dbg)
        printf("wsize: %dx%d\n", wsize.ws_row, wsize.ws_col);
        */

        int dafcols = wsize.ws_col / (maxl + 9);

        /* 4dbg
        printf("dafcols: %d\n", dafcols);
        printf("\n---------------------------\n\n");
        */

        if (dafcols * wsize.ws_row >= dafsCount)
        {
            for (int i = 0; i < wsize.ws_row; i++)
            {
                for (int j = 0; j < dafcols; j++)
                {
                    if (i + j * wsize.ws_row >= dafsCount)
                    {
                        printf("\n");
                        break;
                    }
                    printf("%2d - %-*s    ", i + j * wsize.ws_row, maxl, dafs[i + j * wsize.ws_row]);
                }
            }
        }

        char *command;
        command = (char *)malloc(sizeof(""));
        strcpy(command, "");
        while (1)
        {
            char c = getchar();
            if (c != 0x0A)
            {
                command = (char *)realloc(command, sizeof(command) + sizeof(c));
                char toBeConcatnated[2] = {c, '\0'};
                strcat(command, toBeConcatnated);
            }
            else
            {
                putchar('\b');
                putchar('\0');
                putchar('\b');
                break;
            }
        }

        int jmpIdx;
        char *toJmp;
        if (sscanf(command, "%d", &jmpIdx))
        {
            path = (char *)realloc(path, sizeof(path) + sizeof(dafs[jmpIdx]));
            strcat(path, dafs[jmpIdx]);
        }
        else if (sscanf(command, "%s", toJmp))
        {
            if (!strncmp(toJmp, ":", 1))
            {
                char *extend = toJmp + 1;
                if (!strcasecmp(extend, "quit") || !strcasecmp(extend, "q"))
                {
                    return 0;
                }
                if (!strcasecmp(extend, "show") || !strcasecmp(extend, "s"))
                {
                    printf("\x1B[A\x1B[2K\r");
                    printf("Current path: %s [press any key to continue]", path);
                    getchar();
                    continue;
                }
                printf("\x1B[A\x1B[2K\r");
                printf("Unknown command: %s [press any key to continue]", extend);
                getchar();
                continue;
            }
            int found = 0;
            for (int i = 0; i < dafsCount; i++)
            {
                if (!strcmp(dafs[i], toJmp))
                {
                    path = (char *)realloc(path, sizeof(path) + sizeof(dafs[i]));
                    strcat(path, dafs[i]);
                    found = 1;
                    break;
                }
            }
            if (found) {
                continue;
            }
            else {
                for (int i = 0; i < dafsCount; i++)
                {
                    if (!strncmp(dafs[i], toJmp, strlen(dafs[i]) > strlen(toJmp) ? strlen(toJmp) : strlen(dafs[i])))
                    {
                        path = (char *)realloc(path, sizeof(path) + sizeof(dafs[i]));
                        strcat(path, dafs[i]);
                        break;
                    }
                }
            }
        }
        
    }
}
