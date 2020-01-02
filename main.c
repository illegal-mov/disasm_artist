#include <stdio.h>
#include <stdlib.h>
#include "BeaEngine.h"

static const int RET = 0xC3;

/* Print message to stderr and exit with the passed error code
 */
void fatal(const char *str, int err_code)
{
    fprintf(stderr, str);
    printf("\n");
    exit(err_code);
}

/* Read file content into allocated buffer
 */
void* load_file(const char *str, size_t offset, size_t *fsize)
{
    FILE *fp = fopen(str, "rb");
    if (fp == NULL)
        fatal("Unable to open the file", -1);

    // get file size
    fseek(fp, 0, SEEK_END);
    *fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (offset >= *fsize)
        fatal("Offset is greater than file size", -1);

    // allocate memory
    void *bytes = malloc(*fsize);
    if (bytes == NULL)
        fatal("Memory allocation failed", -1);

    // read file content
    if (*fsize != fread(bytes, 1, *fsize, fp))
        fatal("Failed to read file", -1);

    return bytes;
}

/* Disassemble at `offset` until EOF or a `ret` instruction
 */
void disassemble(void *bytes, size_t offset, size_t fsize)
{
    DISASM infos = {.EIP=(uint64_t)(bytes + offset)};

    while (infos.Error == 0 && offset < fsize) {
        int len = Disasm(&infos);
        offset += len;
        if (infos.Error != UNKNOWN_OPCODE) {
            puts(infos.CompleteInstr);
            infos.EIP += len;
            if (infos.Instruction.Opcode == RET)
                break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage: %s <FILE> <OFFSET>\n", argv[0]);
        return 0;
    }

    size_t offset = 0;
    if (sscanf(argv[2], "%lu", &offset) != 1)
        fatal("Offset must be a number", -1);

    size_t fsize = 0;
    void *bytes = load_file(argv[1], offset, &fsize);
    disassemble(bytes, offset, fsize);
    free(bytes);
    return 0;
}
