#ifndef __PARSER_H__
#define __PARSER_H__

#define MAGIC_SIZE 4
#define VERSION_SIZE 4
#define LABEL_SIZE 32
#define IMTB_HEADER_SIZE 52
#define CHUNK_HEADER_SIZE 68
#define HEADER_MAGIC "RIMG"

typedef struct {
    char magic[MAGIC_SIZE];
    unsigned int version;
    unsigned int chunk_size;
    unsigned int total_chunks;
    unsigned int file_size;
    char label[LABEL_SIZE];
} ImtbHeader;

typedef struct {
    unsigned int type;
    unsigned int size;
    unsigned int offset;
    unsigned int part_size;
    unsigned int crc32;
    unsigned int reserved[12];
} ChunkHeader;

int parse_imtb_file(void *data, ChunkHeader *chunk_data);

#endif