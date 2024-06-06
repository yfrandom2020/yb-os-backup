#include <fat16/fat16.h>
#include <util/util.h>
// A cluster is comprised of 16 sectors - 16*512 = 8192 bytes
// This is the beggining of the file system - which is FAT16
// Using GHEX to statically view os-disk.raw
// FAT16 has three main parts: 1) Reserved (vbr inside) 2) FAT and directory entry 3) actual data
// Entry in FAT is 16 bit
// Directory entry is 32 bit


MBR mbr;
FAT16_BootSector boot_sector;
uint32_t lba_start; // start sector of partition
uint32_t lba_limit; // size in sectors of partition
uint8_t partition_type; // 0x0E to indicate FAT16


// Boot sector data
uint16_t bytes_per_sector;
uint8_t sectors_per_cluster;
uint16_t reserved_sectors; // How many sectors before the actual beggining of FAT
uint8_t number_of_FATs; // Usually two copies of FAT table
uint16_t root_entries; // How many root entries - usually ignored (?)
uint16_t total_sectors;

uint16_t sectors_per_FAT; // size of FAT - lba + reserved + number_of_FATs*sectors_per_FAT = directory
uint32_t root_directory_sectors;
uint32_t first_data_sector;
uint32_t total_clusters;
uint32_t fat_start_sector;
uint32_t root_dir_start_sector;


void Read_MBR()
{
    // Write to the first sector of the disk - which is the MBR
    // In: struct sized 512 that includes a partition table
    // Out: None

    ata0m.Read28(0, 512, (uint8_t*)&mbr); // Load MBR into mbr struct, this will actually load correctly the data

    // for (int i = 0; i < 512; i++)
    // {
    //     printfHex(((uint8_t*)&mbr)[i]);
    //     printf((uint8_t*)" ",0);
    // }
    
    
    // if (mbr.signature == 0xaa55) 
    // {
    //     printf((uint8_t*)"no error",0);

    // }
    // else 
    // {
    //     printf((uint8_t*)"error",0);
    // }
    
    MBR_PartitionEntry* firstPartition = &(mbr.partitionTable[0]); // pointer to the first partition table entry

    lba_start = firstPartition->relativeSector; // Actual beggining of our partition value is 2048 in our case 0x800

    lba_limit = firstPartition->totalSectors; // Size of partition

    partition_type = firstPartition->partitionType;


    // // printfHex(partition_type);
    // // printf((uint8_t*)"\n",0);


    // printf((uint8_t*)"lba_limit \n",0);
    // printfHex32(lba_start);
    // printf((uint8_t*)"\n",0);

    // Purpose of function: to set up values in the lba global variables, to reach boot sector  
}


void readBootSector() 
{
    // Implement reading of the boot sector using your disk I/O functions

    // First we read the entire sector into the bootsector structure
    // Then parse it anf gain the useful data
    
    ata0m.Read28(lba_start, sizeof(boot_sector), (uint8_t*)&boot_sector); // Load boot sector

    bytes_per_sector = boot_sector.BPB_BytsPerSec; // 512
    sectors_per_cluster = boot_sector.BPB_SecPerClus; // 16
    reserved_sectors = boot_sector.BPB_RsvdSecCnt; // 16 sectors from start of partition
    number_of_FATs = boot_sector.BPB_NumFATs; // 2
    root_entries = boot_sector.BPB_RootEntCnt; // 512
    total_sectors = boot_sector.BPB_TotSec16; // 0
    if (total_sectors == 0) 
    {
        total_sectors = boot_sector.BPB_TotSec32;
    }
    sectors_per_FAT = boot_sector.BPB_FATSz16; // sectors per fat - there are 2 of them. size is 256 (in sectors)
    root_directory_sectors = ((root_entries * 32) + (bytes_per_sector - 1)) / bytes_per_sector; // size of root directory
    first_data_sector = reserved_sectors + (number_of_FATs * sectors_per_FAT) + root_directory_sectors;
    total_clusters = (total_sectors - first_data_sector) / sectors_per_cluster;

    // Calculate the starting sector of the FAT and root directory - this is the important thing
    fat_start_sector = lba_start + reserved_sectors;
    root_dir_start_sector = fat_start_sector + (number_of_FATs * sectors_per_FAT); // sector 0xA10 which is 0x142000

    // printf((uint8_t*)"bytes per sector \n",0);
    // printfHex16(bytes_per_sector);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)"sectors per cluster \n",0);
    // printfHex(sectors_per_cluster);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)"reserved sectors \n",0);
    // printfHex16(reserved_sectors);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)"number of fats \n",0);
    // printfHex(number_of_FATs);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)"root entries \n",0);
    // printfHex16(root_entries);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)"total sectors \n",0);
    // printfHex16(total_clusters);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)"sectors per fat \n",0);
    // printfHex16(sectors_per_FAT);
    // printf((uint8_t*)"\n",0);


    // printf((uint8_t*)"root directory sectors \n",0);
    // printfHex32(root_directory_sectors);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)"first data sector \n",0);
    // printfHex32(first_data_sector);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)"total clusters \n",0);
    // printfHex32(total_clusters);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)"fat start sector \n",0);
    // printfHex32(fat_start_sector);
    // printf((uint8_t*)"\n",0);

    // printf((uint8_t*)" root dir start sector \n",0);
    // printfHex32(root_dir_start_sector);
    // printf((uint8_t*)"\n",0);    


    //-----------------------------------------------------------
    // printfHex((root_entries >> 8) & 0xFF);
    // printf((uint8_t*)"\n",0);

    // printfHex(root_entries  & 0xFF);
    // printf((uint8_t*)"\n",0);

    // printfHex(sectors_per_cluster);
    // printf((uint8_t*)"\n",0);


    // printfHex(number_of_FATs);
    // printf((uint8_t*)"\n",0);

    // printfHex(boot_sector.BPB_SecPerClus);

    // Purpose of function: to set up global values relating to boot sector

    
}


void List_Entries()
{
    // After we got our variables correctly from Read_MBR and readbootsector we should be able to reach beggining of the FAT and directory entries
    // Go to directory entries and list the entries (files and sub folders) in the current directory

    DirectoryEntry entries[16]; // size 32 * 16 = 512 bytes
    // 32 sectors?

    ata0m.Read28(root_dir_start_sector, 512, (uint8_t*)&entries);

    for (int i = 0; i < 16; i++)
    {
        printf(entries[i].name,0); // This works and prints TXT!
    }


}

void Read_Cluster(uint16_t cluster_number, uint8_t* ptr)
{
    // This function will connect between the read/write interface of sectors to the read/write of clusters
    // We will set ptr in advance, instead of inside function
    uint32_t start_sector = sectors_per_cluster * (cluster_number - 1);
    for (int i = 0; i < sectors_per_cluster; i++) 
    {
        ata0m.Read28(start_sector + i, bytes_per_sector, (uint8_t*)&ptr[i*bytes_per_sector]); // Read the 512 of the sector into the array, move the pointer of the array according to i
    }
}

void Write_Cluster(uint16_t cluster_number, uint8_t* ptr)
{
    // This function will connect between the read/write interface of sectors to the read/write of clusters
    // We will set ptr in advance, instead of inside function
    uint32_t start_sector = sectors_per_cluster * (cluster_number - 1);
    for (int i = 0; i < sectors_per_cluster; i++) 
    {
        ata0m.Write28(start_sector + i, (uint8_t*)&ptr[i*bytes_per_sector], bytes_per_sector); // Read the 512 of the sector into the array, move the pointer of the array according to i
    }
}


uint16_t Read_FAT_Entry(uint16_t cluster) 
{
    uint32_t fat_offset = cluster * 2;
    uint32_t fat_sector = fat_start_sector + (fat_offset / bytes_per_sector);
    uint32_t ent_offset = fat_offset % bytes_per_sector;
    uint16_t entry;
    ata0m.Read28(fat_sector, bytes_per_sector, (uint8_t*)&entry);
    return entry;
}

void Read_File(char* name) {
    DirectoryEntry entries[16];
    ata0m.Read28(root_dir_start_sector, 512, (uint8_t*)&entries);
    for (int i = 0; i < 16; i++) {
        if (strcmp((char*)entries[i].name, name) == 0) {
            uint16_t cluster = entries[i].firstClusterLow;
            uint32_t file_size = entries[i].fileSize;
            uint8_t buffer[8192]; // Assume maximum file size of 8KB for simplicity
            uint32_t bytes_read = 0;
            while (cluster < 0xFFF8 && bytes_read < file_size) {
                Read_Cluster(cluster, buffer + bytes_read);
                bytes_read += sectors_per_cluster * bytes_per_sector;
                cluster = Read_FAT_Entry(cluster);
            }
            printf(buffer, 0); // Print file contents
            return;
        }
    }
    printf((uint8_t*)"File not found.\n", 0);
}