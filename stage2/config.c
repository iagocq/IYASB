#include <config.h>

#include <alloc.h>
#include <fat32.h>
#include <string.h>

raw_config_entry_t expect_entry(char **buffer);

config_result_t read_config(const char *path, config_t *config) {
    if (config == NULL) {
        return CONFIG_INVALID_PARAM;
    }

    file_t *file = fopen(path, "rb");
    if (file == NULL) {
        return CONFIG_FAILED_TO_OPEN;
    }

    size_t size = fsize(file);
    if (size > MAX_CONFIG_SIZE) {
        return CONFIG_FILE_TOO_BIG;
    }

    char  buffer[size + 1];
    char *buffer_ptr = buffer;
    fread(buffer, 1, size, file);
    fclose(file);

    buffer[size] = '\0';

    struct entry_to_find {
        config_entry_t *entry;
        char *          entry_name;
    } entries[] = {{.entry = &config->filename, .entry_name = "filename"},
                   {.entry = &config->cmd_line, .entry_name = "cmd-line"}};

    size_t n_entries = sizeof(entries) / sizeof(entries[0]);

    while (true) {
        raw_config_entry_t cfg_entry = expect_entry(&buffer_ptr);
        if (buffer_ptr == NULL) {
            break;
        }

        size_t entry_len = cfg_entry.entry_name_len;
        size_t value_len = cfg_entry.value_len;

        for (size_t i = 0; i < n_entries; i++) {
            if (!entries[i].entry->filled) {
                if (strncmp(entries[i].entry_name, cfg_entry.entry_name, entry_len) != 0) {
                    continue;
                }

                char *value_holder = alloc(value_len + 1);
                memcpy(value_holder, cfg_entry.value, value_len);
                value_holder[value_len] = '\0';

                entries[i].entry->value  = value_holder;
                entries[i].entry->filled = true;
                break;
            }
        }
    }

    for (size_t i = 0; i < n_entries; i++) {
        if (!entries[i].entry->filled) {
            return CONFIG_MISSING_ENTRIES;
        }
    }

    return CONFIG_OK;
}

raw_config_entry_t expect_entry(char **buffer) {
    char *real_buffer = *buffer;

    // Skip leading whitespaces
    while (isspace(*real_buffer)) {
        real_buffer++;
    }

    char *line_end = strchr(real_buffer, '\n');

    raw_config_entry_t cfg_entry = {0};

    // Skip comments
    //  - comments can only be on their own line
    if (*real_buffer == '#') {
        *buffer = line_end;
        return cfg_entry;
    }

    char *entry_start = real_buffer;
    char *entry_end   = entry_start;

    // Find the end of the entry name
    while (isalphanum(*entry_end)) {
        entry_end++;
    }

    // Find =
    real_buffer = strchr(entry_end, '=');
    if (real_buffer == NULL) {
        *buffer = line_end;
        return cfg_entry;
    }

    real_buffer++;

    while (isspace(*real_buffer)) {
        real_buffer++;
    }

    char *value_start = real_buffer;
    char *value_end   = value_start;

    // The value only ends on a new line
    while (*value_end != '\n' && *value_end != '\0') {
        value_end++;
    }

    // Remove trailing spaces from value
    while (isspace(*value_end)) {
        value_end--;
    }

    value_end += 1;

    cfg_entry.entry_name     = entry_start;
    cfg_entry.entry_name_len = entry_end - entry_start;

    cfg_entry.value     = value_start;
    cfg_entry.value_len = value_end - value_start;

    *buffer = line_end;

    return cfg_entry;
}
