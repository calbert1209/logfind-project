#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dbg.h"

int isBlankLine(char* string)
{
    int i = 0, j = strlen(string);
    if (j <= 0)
    {
        return -1;
    }

    for (i = 0; i < (j - 1); i++ )
    {
        if (isblank(string[i]) == 0)
        {
            return 0;
        }
    }
    return 1;
}

int countlines(FILE* fstream)
{
    int ch = 0;
    int count = 0;
    while(EOF != (ch = getc(fstream)))
    {
        if (ch == '\n')
        {
            count++;
        }
    }
    return count;
}

int freadlines(char* filename, int* linec, char** lines)
{
    FILE* fileStream = fopen(filename, "r");
    check_mem(fileStream);

    * linec = countlines(fileStream);
    char ** linesOut = malloc(sizeof(char *) * (*linec));
    check_mem(linesOut);

    int i = 0;
    const int LINE_MAX = 256;
    log_info("ln 50")
    while(1)
    {
        char* line = fgets(line, LINE_MAX, fileStream);
        if (line == NULL)
        {
            break;
        }
        printf("freadline: %d: %s", i, line);
        linesOut[i] = line;
        i++;
    }

    lines = linesOut;
    fclose(fileStream);
    // free(linesOut);
    return 1;

    error:
        fclose(fileStream);
        // free(linesOut);
        return -1;
}


int main(int argc, char* argv[])
{
    
    // open .logfind file, get non-blank,non-comment lines as filepaths
    // FILE* fileStream = fopen(".logfind", "r");
    // check_mem(fileStream);

    // const int LINE_MAX = 256;
    
    int lineCount;
    char** lines = NULL;
    int result = freadlines(".logfind", &lineCount, lines);
    log_info("result: %d", result);
    // check_mem(lines);
    int i = 0;
    for (i = 0; i < lineCount; i++)
    {
        printf("%d: %s", i, lines[i]);

    }
    // int cnt = 0;
    
    // while(1)
    // {
    //     char* line = fgets(line, LINE_MAX, fileStream);
    //     if (line == NULL)
    //     {
    //         printf("\n");
    //         break;
    //     }
    //     int isComment = line[0] == '#' ? 1 : 0;
    //     if (isComment == 0 && isBlankLine(line) == 0)
    //     {
    //         printf("%d: (%lu) %s", cnt, strlen(line), line);
    //     }
    //     cnt++;
    // }


    // fclose(fileStream);
    // return string array from .logfind file
    free(lines);
    return 0;

    error:
        // fclose(fileStream);
        free(lines);
        printf("error.");
        return -1;
}