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

    // stripped is the original value of inputCopy
    free(stripped);
    return 1;

    error:
        return -1;
}

int GetFileCharCount(FILE* file, long* charCount)
{
    fseek(file, 0, SEEK_END);
    *charCount = ftell(file);
    rewind(file);

    return 0;
}

int CopyFileToBuffer(FILE* file, long charCount, char** buffer)
{
    if (!file)
    {
        log_err("invalid file stream");
        return -1;
    }

    if (!(*buffer))
    {
        log_err("invalid buffer");
        return -1;
    }

    fread(*buffer, 1, charCount, file);
    
    return 0;
}

int GetFilePathList(FILE* listStream, char** list, int* pathCount)
{
    char* lineBuffer = malloc(sizeof(char) * LINEMAX);
    check_debug(lineBuffer, "lineBuffer allocation failed");
    int lineCount = 0;

    while(fgets(lineBuffer, LINEMAX, listStream))
    {
        if (CommentCheck(lineBuffer) == 0 && BlankLineCheck(lineBuffer) == 0)
        {
            // for str = {'a', 'b', 'c', '\n', '\0'}, strlen(str) is 4
            int indexOfCarrReturn = strlen(lineBuffer) - 1;
            if(lineBuffer[indexOfCarrReturn] == '\n')
            {
                lineBuffer[indexOfCarrReturn] = '\0';
            }

            char* filePath = strdup(lineBuffer);
            list[lineCount++] = filePath;
        }
    }
    *pathCount = lineCount;

    free(lineBuffer);
    return 0;

    error:
        return -1;
}


// Search Functions
// ====================================================================

int HasAny(char* haystack, int needleCount, char** needles, int offset)
{
    int i = 0;
    for (i = offset; i < needleCount; i++)
    {
        char* searchResult = strstr(haystack, needles[i]);
        if (searchResult)
        {
            printf("FindAny: found %s\n", needles[i]);
            return i;
        }
    }
    return -1;
}

int HasAll(char* haystack, int needleCount, char** needles, int offset)
{
    int i = 0;
    for (i = offset; i < needleCount; i++)
    {
        char* searchResult = strstr(haystack, needles[i]);
        if(searchResult)
        {
            continue;
        }
        return -1;
    }
    printf("FindAll: succeeded\n");
    return 0;
}

int ExecuteSearch(char* haystack, int needleCount, char** needles, int offset, int hasOr)
{
    int searchResult = -1;
    if (hasOr > 0)
    {
        searchResult = HasAny(haystack, needleCount, needles, offset);
    }
    else
    {
        searchResult = HasAll(haystack, needleCount, needles, offset);
    }
    return searchResult;
}

// Term Parsing
// ====================================================================

int GetOrFlagIndex(int argc, char* argv[])
{
    char* orFlag = "-o";
    int i = 1;
    for (i = 1; i < argc; i++)
    {
        if(strcmp(orFlag, argv[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

int GetSearchTerms(int argc, char* argv[], char* terms[])
{
    // NOTE what happens if block for terms is too short for args?
    int i = 1;
    int last = 0;
    for (i = 1; i < argc; i++)
    {
        if(strcmp("-o", argv[i]) != 0)
        {
            terms[last] = argv[i];
            last++;
        }
    }
    return 0;
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


int main(int argc, char* argv[])
{
    // Prep search terms

    if(argc <= 1)
    {
        printf("logfind needs at least one query term!\n");
    }

    int OrFlagIndex = GetOrFlagIndex(argc, argv);
    int termCount = -1;

    if(OrFlagIndex >= 0)
    {
        termCount = argc - 2;
    }
    else
    {
        termCount = argc - 1;
    }
    
    char** terms = malloc(sizeof(char*) * termCount);
    check_debug(terms, "could not allocate for terms");
    GetSearchTerms(argc, argv, terms);

    // === Load List of Log Paths
    FILE* pathListStream = fopen(".logfind", "r");
    check_mem(pathListStream);

    int pathListStreamLineCount = CountLines(pathListStream);
    char** pathList = calloc(pathListStreamLineCount, sizeof(char*));
    check_debug(pathList, "could not allocate pathList");
    int pathCount = -1;
    GetFilePathList(pathListStream, pathList, &pathCount);

    // DEBUG
    long logFileLength = 0;     
    int x = 0;
    for(x = 0; x < pathCount; x++)
    {
        FILE* logFileStream = fopen(pathList[x], "r");
        if(!logFileStream)
        {
            log_warn("could not open %s", pathList[x]);
            continue;
        }

        GetFileCharCount(logFileStream, &logFileLength);
        char* logFileBuffer = malloc(sizeof(char) * logFileLength + 1);
        CopyFileToBuffer(logFileStream, logFileLength + 1, &logFileBuffer);
        logFileBuffer[logFileLength + 1] = '\n';
        int searchResult = ExecuteSearch(logFileBuffer, termCount, terms, 0, OrFlagIndex);
        // log_info("%d: %s (%lu bytes)", x, pathList[x], (sizeof(char) * logFileLength));
        if (searchResult >= 0)
        {
            log_info("search match in %s", pathList[x]);
        }

        fclose(logFileStream);
    }



    // === clean up
    // TODO free all strings in path list
    free(terms);
    fclose(pathListStream);

    return 0;

    error:
        printf("error.\n");
        return -1;
}

/*
    // === Load List of Logs

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
*/