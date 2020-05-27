#pragma once

#include <fat32.h>

#include <stddef.h>

#ifndef MAX_CONFIG_SIZE
#define MAX_CONFIG_SIZE 2048
#endif

typedef struct config_entry {
    bool  filled;
    char *value;
} config_entry_t;

typedef struct config {
    config_entry_t filename;
    config_entry_t cmd_line;
} config_t;

typedef struct raw_config_entry {
    char * entry_name;
    size_t entry_name_len;

    char * value;
    size_t value_len;
} raw_config_entry_t;

typedef enum config_result {
    CONFIG_OK = 0,
    CONFIG_FILE_TOO_BIG,
    CONFIG_FAILED_TO_OPEN,
    CONFIG_INVALID_PARAM,
    CONFIG_MISSING_ENTRIES
} config_result_t;

config_result_t read_config(const char *path, config_t *config);
