
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

int main(int argc, char *argv[])
{
    FILE *fp;
    char *source;
    long file_size;
    size_t bytes_read;

    if (argc != 2)
    {
        printf("Usage: %s input.c\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "rb");

    if (fp == NULL)
    {
        perror("File opening failed");
        return 1;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        printf("File positioning failed\n");
        fclose(fp);
        return 1;
    }

    file_size = ftell(fp);

    if (file_size < 0)
    {
        printf("Unable to determine file size\n");
        fclose(fp);
        return 1;
    }

    rewind(fp);

    source = malloc((size_t)file_size + 1);

    if (source == NULL)
    {
        printf("Memory allocation failed\n");
        fclose(fp);
        return 1;
    }

    bytes_read = fread(source, 1, (size_t)file_size, fp);

    if (bytes_read != (size_t)file_size)
    {
        printf("File reading failed\n");
        free(source);
        fclose(fp);
        return 1;
    }

    source[file_size] = '\0';

    fclose(fp);

    scan_source(source);

    free(source);

    return 0;
}
