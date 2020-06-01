#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "disk.h"

#define xcat(a, b) a##b
#define cat(a, b)  xcat(a, b)

#define RESOURCE_FULL(res, suffix) cat(cat(_resource_, res), cat(_, suffix))

#define EXTERN_RESOURCE(res, suffix) extern char RESOURCE_FULL(res, suffix)
#define DEFINE_RESOURCE(res)     \
    EXTERN_RESOURCE(res, start); \
    EXTERN_RESOURCE(res, end)

#define GET_RESOURCE(res, suffix) (RESOURCE_FULL(res, suffix))
#define RESOURCE_ENTRY(res)                                     \
    extern_resources[cat(RESOURCE_, res)] = (resource_data_t) { \
        &GET_RESOURCE(res, start), &GET_RESOURCE(res, end),     \
            &GET_RESOURCE(res, end) - &GET_RESOURCE(res, start) \
    }

typedef struct resource_data {
    void *start;
    void *end;
    int   size;
} resource_data_t;

DEFINE_RESOURCE(bootsector_bin);
DEFINE_RESOURCE(stage2_bin);

typedef enum resource { RESOURCE_bootsector_bin, RESOURCE_stage2_bin, RESOURCE_N } resource_t;
resource_data_t extern_resources[RESOURCE_N];

void initiate_resources() {
    RESOURCE_ENTRY(bootsector_bin);
    RESOURCE_ENTRY(stage2_bin);
}

resource_data_t get_resource(resource_t resource) {
    if (resource >= RESOURCE_N) {
        resource = RESOURCE_N;
    }

    return extern_resources[resource];
}

void usage(char *argv[]);

char example_config[] = {"# This is an example configuration file\n"
                         "\n"
                         "# Change this path to the kernel image\n"
                         "filename = /boot/kernel\n"
                         "\n"
                         "# Add something to this line if there should be\n"
                         "# a command line passed to the kernel\n"
                         "cmd-line =\n"};

int main(int argc, char *argv[]) {
    initiate_resources();

    resource_data_t bootsector = get_resource(RESOURCE_bootsector_bin);
    resource_data_t stage2     = get_resource(RESOURCE_stage2_bin);

    assert(bootsector.size == sizeof(fat32_bootsector_t));
    assert(stage2.size > 0);

    if (argc < 3) {
        usage(argv);
        return 2;
    }

    char *filename = argv[1];
    char *root     = argv[2];

    FILE *bs_file = fopen(filename, "r+b");
    assert(bs_file != NULL);

    fat32_bootsector_t new_bootsector, bootsector_to_replace;
    memcpy(&new_bootsector, bootsector.start, bootsector.size);

    size_t read = fread(&bootsector_to_replace, sizeof(fat32_bootsector_t), 1, bs_file);
    assert(read == 1);
    assert(bootsector_to_replace.boot_signature == 0xAA55);

    memcpy(bootsector_to_replace.jmp, new_bootsector.jmp, sizeof(new_bootsector.jmp));
    memcpy(bootsector_to_replace.boot_code,
           new_bootsector.boot_code,
           sizeof(new_bootsector.boot_code));

    fseek(bs_file, 0, SEEK_SET);
    size_t written = fwrite(&bootsector_to_replace, sizeof(fat32_bootsector_t), 1, bs_file);
    assert(written == 1);

    fclose(bs_file);

    size_t root_len = strlen(root);

    char stage2_path[root_len + 7 + 1];
    strcpy(stage2_path, root);
    strcat(stage2_path, "/stage2");

    FILE *stage2_file = fopen(stage2_path, "wb");
    assert(stage2_file != NULL);

    written = fwrite(stage2.start, stage2.size, 1, stage2_file);
    assert(written == 1);

    fclose(stage2_file);

    char config_path[root_len + 5 + 10 + 1];
    strcpy(config_path, root);
    strcat(config_path, "/boot/iyasb.cfg");

    FILE *config_file = fopen(config_path, "r");
    if (config_file == NULL) {
        config_file = fopen(config_path, "wb");
        assert(config_file != NULL);

        written = fwrite(example_config, sizeof(example_config), 1, config_file);
        assert(written == 1);
    }
    fclose(config_file);

    printf("Done!\n");
    printf("Remember to sync(3) the filesystem *and* the device\n");

    return 0;
}

void usage(char *argv[]) {
    fprintf(stderr, "Usage: %s file root\n", argv[0]);
}
