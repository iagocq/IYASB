#pragma once
#include <stdint.h>

typedef struct __attribute__((__packed__)) partition {
    uint8_t  bootable;
    uint8_t  start_addr[3];
    uint8_t  type;
    uint8_t  end_addr[3];
    uint32_t lba_start;
    uint32_t sectors_n;
} partition_t;

typedef struct __attribute__((__packed__)) mbr {
    uint8_t     bootstrap[440];
    uint32_t    disk_id;
    uint16_t    a;
    partition_t partitions[4];
    uint16_t    signature;
} mbr_t;

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

typedef struct __attribute__((__packed__)) fat32_bootsector {
    uint8_t     jmp[3];
    uint8_t     oem_name[8];
    fat32_bpb_t fat32_bpb;
    uint8_t     boot_code[419];
    uint8_t     old_pdrive;
    uint16_t    boot_signature;
} fat32_bootsector_t;

typedef struct __attribute__((__packed__)) mbr_bootsector {
    union {
        uint8_t boot_code[440];
        struct __attribute__((__packed__)) {
            uint8_t  boot_code_1[218];
            uint16_t zero;
            uint8_t  pdrive;
            uint8_t  seconds;
            uint8_t  minutes;
            uint8_t  hours;
            uint8_t  boot_code_2[216];
        };
    };
    union {
        uint8_t eboot_code[6];
        struct __attribute__((__packed__)) {
            uint32_t signature;
            uint16_t copy_protected;
        };
    };
    partition_t partitions[4];
    uint16_t    boot_signature;
} mbr_bootsector_t;

typedef enum { FAT32_BOOTSECTOR, MBR_BOOTSECTOR } bootsector_type;

typedef struct bootsector {
    bootsector_type bs_type;
    union {
        fat32_bootsector_t fat32_bs;
        mbr_bootsector_t   mbr_bs;
    };
} bootsector_t;

void init_disk(uint8_t disk_number);
void read_sector(uint32_t sector, void *buffer);

extern uint32_t bytes_per_sector;
