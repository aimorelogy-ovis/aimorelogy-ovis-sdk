#include <stdio.h>
#include <string.h>
#include "parser.h"

int parse_imtb_file(void *data, ChunkHeader *chunk_data)
{
    if (!data) {
        printf("Failed to load imtb data");
        return -1;
    }

    int ret = -1;

    char *buffer = (char *)(*(unsigned long *)data);
    ChunkHeader *chunk = NULL;

    for(int i=0;i<8;i++)
    {
        ImtbHeader *header = (ImtbHeader *)(buffer + i * (IMTB_HEADER_SIZE+CHUNK_HEADER_SIZE));

        if (strncmp(header->magic, HEADER_MAGIC, 4) != 0) {
            printf("Invalid magic\n");
            continue;
        }

        for (int j = 0; j < header->total_chunks; j++) {
            chunk = (ChunkHeader *)(buffer + sizeof(ImtbHeader)
                                + j * sizeof(ChunkHeader)
                                + i * (IMTB_HEADER_SIZE+CHUNK_HEADER_SIZE));
        }

        if ((strncmp(header->label, "PRIM", 4) == 0) && !(strncmp(header->label+4, "0", 1) == 0)) {
            printf("Found imtb.\n");
            memcpy(chunk_data, chunk, sizeof(ChunkHeader));
            ret = 0;
            break;
        }

    }

    return ret;
}
