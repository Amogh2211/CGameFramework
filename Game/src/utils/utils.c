#include "utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char* readFileIntoString(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    assert(file);
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* data = (char*)malloc(length + 1);
    assert(data);
    if (data)
    {
        fread(data, 1, length, file);
        data[length] = '\0';
        fclose(file);
        return data;
    }

    return NULL;
}

