#pragma once
#include <types.h>
#include <fat16/disk.h>
#include <util/util.h>
extern ata ata0m;
typedef struct 
{
    // The mbr contains three different parts
    // Boot code - first 446 bytes
    // Partition tables - 4 entries with each sized 16 bytes that describe different partitions on the disk 
    // The struct describes a single entry
    uint8_t  bootIndicator;
    uint8_t  startHead;
    uint8_t  startSector;
    uint8_t  startCylinder;
    uint8_t  partitionType;
    uint8_t  endHead;
    uint8_t  endSector;
    uint8_t  endCylinder;
    uint32_t relativeSector; // 00 08 00 00 is loaded - exactly like disk analysis
    uint32_t totalSectors; // 00 f8 0f 00
} __attribute__((packed)) MBR_PartitionEntry;


typedef struct 
{
    // Implement mbr strcure, we will load this into sector 0
    uint8_t  bootstrap[446];
    MBR_PartitionEntry partitionTable[4];
    uint16_t signature; // 0xAA55
} __attribute__((packed)) MBR;


typedef struct 
{
    // The first sector of each partition is called the boot sector and contains some metadata about the partition and the file system
    uint8_t  BS_jmpBoot[3]; // jmp instruction
    uint8_t  BS_OEMName[8]; // name mkfs.fat
    uint16_t BPB_BytsPerSec; // 512
    uint8_t  BPB_SecPerClus; // 16 0x10
    uint16_t BPB_RsvdSecCnt; // Reserved sector count -1
    uint8_t  BPB_NumFATs; // usually 2
    uint16_t BPB_RootEntCnt; // usually 512?
    uint16_t BPB_TotSec16; // 0
    uint8_t  BPB_Media; // F8 - to signafy sector end
    uint16_t BPB_FATSz16; // fat size - 256
    uint16_t BPB_SecPerTrk; // sectors per track
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    
    uint32_t BPB_TotSec32;
    uint8_t  BS_DrvNum;
    uint8_t  BS_Reserved1;
    uint8_t  BS_BootSig;
    uint32_t BS_VolID;
    uint8_t  BS_VolLab[11];
    uint8_t  BS_FilSysType[8];
} __attribute__((packed)) FAT16_BootSector;


typedef struct 
{
    uint8_t name[8];          // 8 bytes for the file name
    uint8_t extension[3];     // 3 bytes for the file extension
    uint8_t attribute;        // 1 byte for file attributes
    uint8_t reserved;         // 1 byte reserved (unused)
    uint8_t createTimeTenth;  // 1 byte for creation time (tenths of a second)
    uint16_t createTime;      // 2 bytes for creation time
    uint16_t createDate;      // 2 bytes for creation date
    uint16_t lastAccessDate;  // 2 bytes for last access date
    uint16_t firstClusterHigh; // 2 bytes for high word of the first cluster (FAT32)
    uint16_t writeTime;       // 2 bytes for last write time
    uint16_t writeDate;       // 2 bytes for last write date
    uint16_t firstClusterLow; // 2 bytes for low word of the first cluster
    uint32_t fileSize;        // 4 bytes for file size in bytes
} __attribute__((packed)) DirectoryEntry;


typedef struct
{
    uint8_t name[9]; // Including terminator character
    uint16_t firstClusterLow;
    uint32_t size;
} __attribute__((packed)) helper_entry_struct; // Will be used to save values of name and first cluster low for each directory entry



void Read_MBR();
void readBootSector(); // Use ata_read_sector  
FAT16_BootSector parseBootSector(uint8_t* bootSectorBuffer); // Receive the data from previous function and parse
void List_Entries();
void Read_File(uint8_t* name);
void Read_Cluster(uint16_t cluster_number, uint8_t* ptr);
void Write_Cluster(uint16_t cluster_number, uint8_t* ptr);
uint16_t Read_FAT_Entry(uint16_t cluster);

// MBR information about our FAT16 partition
extern uint32_t lba_start;
extern uint32_t lba_limit;
extern uint8_t partition_type;

// Boot sector information about FAT16
extern uint16_t bytes_per_sector;
extern uint8_t sectors_per_cluster;
extern uint16_t reserved_sectors;
extern uint8_t number_of_FATs;
extern uint16_t root_entries;
extern uint16_t total_sectors;


extern uint16_t sectors_per_FAT;
extern uint32_t root_directory_sectors;
extern uint32_t first_data_sector;
extern uint32_t total_clusters;
// Calculate the starting sector of the FAT and root directory
extern uint32_t fat_start_sector;
extern uint32_t root_dir_start_sector;

extern uint16_t sectors_per_FAT;
extern uint32_t root_directory_sectors;
extern uint32_t first_data_sector;
extern uint32_t total_clusters;
// Calculate the starting sector of the FAT and root directory
extern uint32_t fat_start_sector;
extern uint32_t root_dir_start_sector;