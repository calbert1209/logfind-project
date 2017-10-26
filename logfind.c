#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "dbg.h"

int countlines(FILE* fstream)
{
    int ch = 0;
    int count = 0;
    int pos = 0;
    int lastNewLinePos = 0;
    while(EOF != (ch = getc(fstream)))
    {
        pos++;
        if (ch == '\n')
        {
            lastNewLinePos = pos;
            count++;
        }
    }

    if (pos > lastNewLinePos)
    {
        count++;
    }

    rewind(fstream);
    return count;
}

int CommentCheck(char * line)
{
    if(line[0] == '#')
    {
        return 1;
    }

    return 0;
}

int BlankLineCheck(char* line)
{
    int i = 0;
    while(line[i] != '\n')
    {
        if (!isblank(line[i]))
        {
            return 0;
        }
    }

    return 1;
}

int SplitLines(FILE* fstream, int lineCount, char*** splitLines)
{
    char** lineptrs = malloc(sizeof(char*) * lineCount);

    char* current = NULL;
    int i = 0;

    while(fgets(current, 256, fstream))
    {
        lineptrs[i] = current;
        i++;
    }

    // the pointers collected in lineptrs point to memory on the stack...
    // memory that is gonzo when the block completes. try again...
    splitLines = &lineptrs;
    return 1;
}

int main(int argc, char* argv[])
{
    FILE* fstream = fopen(".logfind", "r");
    check_mem(fstream);

    int lineCount = countlines(fstream);
    log_info("number of lines: %d", lineCount);

    char line[256];
    int cnt = 0;

    while(fgets(line, 256, fstream ))
    {
        if (CommentCheck(line) == 0 && BlankLineCheck(line) == 0)
        {
            printf("%d: %s", cnt, line);
            cnt++;
        }
    }
    printf("\n");

    fclose(fstream);
    return 0;

    error:
        printf("error.\n");
        return -1;
}