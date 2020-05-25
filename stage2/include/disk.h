#pragma once
#include <stdint.h>

/** Common FAT32 BPB format */
typedef struct __attribute__((__packed__)) fat32_bpb {
    uint16_t bytes_per_lsector;
    uint8_t  lsectors_per_cluster;
    uint16_t reserved_lsectors;
    uint8_t  num_fat;
    uint16_t root_entries;
    uint16_t old_total_lsectors;
    uint8_t  media_type;
    uint16_t old_lsectors_per_fat;

    uint16_t psectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_lsectors;

    uint32_t lsectors_per_fat;
    uint16_t mirror_flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fs_info_lsector;
    uint16_t bs_backup_lsector;
    uint8_t  reserved_1[12];
    uint8_t  old_pdrive;
    uint8_t  filler;
    uint8_t  old_boot_signature;

    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];

} fat32_bpb_t;

/** Common FAT32 bootsector structure */
typedef struct __attribute__((__packed__)) fat32_bootsector {
    uint8_t     jmp[3];
    uint8_t     oem_name[8];
    fat32_bpb_t fat32_bpb;
    uint8_t     boot_code[419];
    uint8_t     old_pdrive;
    uint16_t    boot_signature;
} fat32_bootsector_t;

void init_disk(uint8_t disk_number);
void read_sector(uint32_t sector, void *buffer);

extern uint32_t bytes_per_sector;
