#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <regex.h>
#include <errno.h>
#include "dbg.h"

const int LINEMAX = 256;

int CountLines(FILE* fstream)
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
        i++;
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

int printFile(FILE* fstream)
{
    char firstLine[LINEMAX];
    fgets(firstLine, LINEMAX, fstream);
    printf("%s\n", firstLine);
    rewind(fstream);
    return 1;
}

int PrintMatches(FILE* stream, regex_t* regex)
{
    char lineBuffer[LINEMAX];
    char msgBuffer[100];
    while(fgets(lineBuffer, LINEMAX, stream))
    {
        int matchResult = regexec(regex, lineBuffer, 0, NULL, 0);
        if(!matchResult)
        {
            printf("match: %s", lineBuffer);
        }
        else if (matchResult == REG_NOMATCH)
        {

        }
        else
        {
            regerror(matchResult, regex, msgBuffer, sizeof(msgBuffer));
            printf("regex failed: %s\n", msgBuffer);
        }
    }
    rewind(stream);
    return 1;
}

// TODO FindAll
// TODO FindAny
// TODO ExecuteFind, finds terms in buffer using strategy based on flag parameter


int main(int argc, char* argv[])
{
    if(argc <= 1)
    {
        printf("logfind needs at least one query term!\n");
    }

    // TODO evaluate args for -o
    // TODO get terms list for -o strategy

    // === Load List of Logs
    FILE* fstream = fopen(".logfind", "r");
    check_mem(fstream);

    int lineCount = CountLines(fstream);
    log_info("number of lines: %d", lineCount);

    char* line = malloc(LINEMAX);
    check_debug(line, "line not allocated");
    int cnt = 0;

    char* rawLine = malloc(LINEMAX);
    check_debug(rawLine, "rawLine note created");

    char* logFilePath = malloc(LINEMAX);

    // === Loop through list of filepaths
    while(fgets(line, LINEMAX, fstream ))
    {
        if (CommentCheck(line) == 0 && BlankLineCheck(line) == 0)
        {
            // == clean up filepath string            
            strcpy(rawLine, line);

            RemoveLineReturn(rawLine, logFilePath);
            
            // == open file stream
            FILE* logStream = fopen(logFilePath, "r");
            check_debug(logStream, "logStream not opened")

            // TODO convert filestream to buffer
            // TODO pass buffer, terms, and strategy flag to ExecuteFind
            // TODO record? report? results of search for filepath

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