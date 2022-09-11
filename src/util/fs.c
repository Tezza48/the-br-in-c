#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char *readFileToString(const char *path)
{
    FILE *file = fopen(path, "r");

    assert(file != 0);

    fseek(file, 0, SEEK_END);

    int64_t numBytes = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *text = calloc(numBytes, sizeof(char));

    fread(text, sizeof(char), numBytes, file);

    fclose(file);

    return text;
}