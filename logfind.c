#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "dbg.h"

const int LINEMAX = 256;

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

int RemoveLineReturn(char* input, char* output)
{
    char* inputCopy = strdup(input);
    check_mem(inputCopy);
    
    char* stripped = strsep(&inputCopy, "\n");

    strcpy(output, stripped);

    free(stripped);
    return 1;

    error:
        return -1;
}

char** ParseLines(FILE* fstream, int* lineCount)
{
    // buffer for line
    char buffer[256];
    // initial line count
    *lineCount = countlines(fstream);
    // char** on heap with space for ptrs to each line
    char** lineList = malloc((*lineCount) * sizeof(char*));
    // loop through file stream
    int i = 0;
    
    while(fgets(buffer, 256, fstream))
    {
        // duplicate line on heap
        char* lineOnHeap = strdup(buffer);
        // push ptr to duplicate to ptr list on heap at index i
        lineList[i] = lineOnHeap;
    }

    rewind(fstream);
    return lineList;
}

int printFile(FILE* fstream)
{
    char firstLine[LINEMAX];
    fgets(firstLine, LINEMAX, fstream);
    printf("%s\n", firstLine);
    rewind(fstream);
    return 1;
}

int main(int argc, char* argv[])
{
    FILE* fstream = fopen(".logfind", "r");
    check_mem(fstream);

    int lineCount = countlines(fstream);
    log_info("number of lines: %d", lineCount);

    char* line = malloc(LINEMAX);
    check_debug(line, "line not allocated");
    int cnt = 0;

    char* rawLine = malloc(LINEMAX);
    check_debug(rawLine, "rawLine note created");

    char* logFilePath = malloc(LINEMAX);

    while(fgets(line, LINEMAX, fstream ))
    {
        if (CommentCheck(line) == 0 && BlankLineCheck(line) == 0)
        {
            strcpy(rawLine, line);

            RemoveLineReturn(rawLine, logFilePath);
            
            FILE* logStream = fopen(logFilePath, "r");
            check_debug(logStream, "logStream not opened")

            printf("title: %s\n", logFilePath);
            printFile(logStream);

            fclose(logStream);
            
            cnt++;
        }
    }
    
    free(rawLine);
    free(logFilePath);
    fclose(fstream);
    free(line);
    return 0;

    error:
        printf("error.\n");
        return -1;
}