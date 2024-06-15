#include <fat16/fat16.h>
#include <util/util.h>
// A cluster is comprised of 16 sectors - 16*512 = 8192 bytes
// This is the beggining of the file system - which is FAT16
// Using GHEX to statically view os-disk.raw
// FAT16 has three main parts: 1) Reserved (vbr inside) 2) FAT and directory entry 3) actual data
// Entry in FAT is 16 bit
// Directory entry is 32 bit
// To read - parse directory entry for cluster and iterate cluster table
// To write - find first position available

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

    
    MBR_PartitionEntry* firstPartition = &(mbr.partitionTable[0]); // pointer to the first partition table entry

    lba_start = firstPartition->relativeSector; // Actual beggining of our partition value is 2048 in our case 0x800

    lba_limit = firstPartition->totalSectors; // Size of partition

    partition_type = firstPartition->partitionType;

    // Purpose of function: to set up values in the lba global variables, to reach boot sector  
}


void readBootSector() 
{
    // Implement reading of the boot sector using your disk I/O functions

    // First we read the entire sector into the bootsector structure
    // Then parse it anf gain the useful data
    
    ata0m.Read28(lba_start, sizeof(boot_sector), (uint8_t*)&boot_sector); // Load boot sector

    // Looking at static hex analysis of os-disk.raw and verifying values, values in comments appear in file analysis
    
    bytes_per_sector = boot_sector.BPB_BytsPerSec; // 512
    
    sectors_per_cluster = boot_sector.BPB_SecPerClus; // 16
    
    reserved_sectors = boot_sector.BPB_RsvdSecCnt; // 16 sectors from start of partition until start of FAT = 0x10
    
    number_of_FATs = boot_sector.BPB_NumFATs; // 2
    
    root_entries = boot_sector.BPB_RootEntCnt; // 512 entries where each is 32 bit = 0x200
    
    total_sectors = boot_sector.BPB_TotSec16; // 0
    
    if (total_sectors == 0) 
    {
        total_sectors = boot_sector.BPB_TotSec32;
    }
    
    sectors_per_FAT = boot_sector.BPB_FATSz16; // 256 = 0x100
    
    fat_start_sector = lba_start + reserved_sectors; // 0x800 + 0x10 = 0x810

   // Apparently the first two clusters in the FAT contain data about EOF symbols
   // Actual fat begins two clusters (32 sectors) after fat_start_sector


    root_dir_start_sector = fat_start_sector + (number_of_FATs * sectors_per_FAT); // Sector 0xA10 which is 0x142000 real address - correct
    
    root_directory_sectors = ((root_entries * 32) + (bytes_per_sector - 1)) / bytes_per_sector; // Size of root directory - 32 sectors = 0x20
    
    first_data_sector = root_dir_start_sector + root_directory_sectors; // We should be getting 0xA30 - 0x146000
    // We need to remember that although the actual data sector begins in 0x146000 the first two clusters are reserved so we need to add 
    // We start from sector 3 instead of sector 2? for no apparent reason - FAT decides to save our first file in the 3rd sector instead of second...
    first_data_sector += 0x10;
    
    total_clusters = (total_sectors - first_data_sector) / sectors_per_cluster;


    if (first_data_sector == 0xA40)
    {
        //printf((uint8_t*)"hi ::",0);
    }
    // Calculate the starting sector of the FAT and root directory - this is the important thing

}

void Read_Cluster(uint16_t cluster_number, uint8_t* ptr)
{
    // This function will connect between the read/write interface of sectors to the read/write of clusters
    // We will set ptr in advance, instead of inside function
    // The sector number given is actually only a sector in the data region
    // For some reason our files begin in sector 3 instead of sector two
    uint32_t start_sector = first_data_sector + ((cluster_number - 3) * sectors_per_cluster);
    for (int i = 0; i < sectors_per_cluster; i++) 
    {
        ata0m.Read28(start_sector + i, bytes_per_sector, (uint8_t*)&ptr[i*bytes_per_sector]); // Read the 512 of the sector into the array, move the pointer of the array according to i
    }
}

void Write_Cluster(uint16_t cluster_number, uint8_t* ptr)
{
    // This function will connect between the read/write interface of sectors to the read/write of clusters
    // We will set ptr in advance, instead of inside function
    uint32_t start_sector =  first_data_sector + ((cluster_number - 3) * sectors_per_cluster);
    for (int i = 0; i < sectors_per_cluster; i++) 
    {
        ata0m.Write28(start_sector + i, (uint8_t*)&ptr[i*bytes_per_sector], bytes_per_sector); // Read the 512 of the sector into the array, move the pointer of the array according to i
        ata0m.Flush();
    }
}

void test_disk()
{
    // Test read and write cluster functions
    //printf((uint8_t*)"entereed here \n",0);
    uint8_t ptr[bytes_per_sector * sectors_per_cluster]; // Size of one cluster exactly
    for (int i = 0; i < bytes_per_sector * sectors_per_cluster; i++)
    {
        ptr[i] = 'A';
        if (i==512) ptr[i] = 'C';
    }

    Write_Cluster(3, ptr);

    uint8_t ptr2[bytes_per_sector * sectors_per_cluster];

    Read_Cluster(3, ptr2);

    //printf(ptr2,0);

    //printf((uint8_t*)"ended successfuly \n",0);
}

helper_entry_struct* file_names()
{
    // Get array of file names from the root directory for now

    static DirectoryEntry entries[16]; // Assuming 16 entries per sector
    ata0m.Read28(root_dir_start_sector, sizeof(entries), (uint8_t*)&entries);

    static helper_entry_struct filenames[256]; // An array of 256 helper_entry_struct
    static uint8_t names_buffer[256][9]; // Buffer to hold up to 256 filenames, each up to 8 characters + null terminator
    int l = 0;

    for (int i = 0; i < 16; i++)
    {
        if (entries[i].name[0] == 0x00)
        {
            // No more entries, exit loop
            break;
        }
        else if (entries[i].name[0] == 0xE5)
        {
            // Entry is deleted, skip to next entry
            continue;
        }

        // Combine name fields to form complete file name
        int k = 0;
        for (int j = 0; j < 8; j++)
        {
            if (entries[i].name[j] != ' ')
            {
                names_buffer[l][k++] = entries[i].name[j];
            }
            else
            {
                break; // Stop copying on first space
            }
        }
        names_buffer[l][k] = '\0'; // Null-terminate the string

        // Copy the file name to the helper_entry_struct
        for (int j = 0; j < 9; j++) {
            filenames[l].name[j] = names_buffer[l][j];
        }
        filenames[l].firstClusterLow = entries[i].firstClusterLow;
        filenames[l].size = entries[i].fileSize;

        l++;
    }

    // Return the array of structs
    return filenames;
}

void List_Entries()
{
    DirectoryEntry entries[16]; 
    // Assuming 16 entries per sector
    // Read the entries found
    ata0m.Read28(root_dir_start_sector, sizeof(entries), (uint8_t*)&entries);

    for (int i = 0; i < 16; i++)
    {
        if (entries[i].name[0] == 0x00) 
        {
            // No more entries, exit loop
            break;
        } 
        else if (entries[i].name[0] == 0xE5) 
        {
            // Entry is deleted, skip to next entry
            continue;
        }

        // Combine name and extension fields to form complete file name
        uint8_t filename[13]; // 8 chars for name + 1 for dot + 3 chars for extension + 1 for null terminator
        int k = 0;
        uint8_t only_name[9];
        for (int j = 0; j < 8; j++) 
        {
            if (entries[i].name[j] != ' ') 
            {
                filename[k] = entries[i].name[j];
                only_name[k] = entries[i].name[j];
                k++;
            }
        }
        only_name[k] = '\0';
        filename[k] = '.';
        k++;
        for (int j = 0; j < 3; j++) 
        {
            if (entries[i].extension[j] != ' ') 
            {
                filename[k++] = entries[i].extension[j];
            }
        }
        filename[k] = '\0'; // Null-terminate the string
        //printf((uint8_t*)"The file name is: \n",0);
        
        if (only_name[2] != '\0' && only_name[2] != ' ')
        {
            //printf((uint8_t*)"\n sending",0);
            //printf((uint8_t*)"\n",0);
            printf(only_name, 0);
            printf((uint8_t*)" ",0);
            //Read_File(only_name);
        }

    }
}

// uint16_t Read_FAT_Entry(uint16_t cluster) 
// {
//     // Get FAT entry, return the value inside the entry
//     // For example read value of cluster 4
//     printf((uint8_t*)"Inside Read_FAT_Entry \n",0);
//     uint32_t fat_offset = cluster * 2;
//     uint32_t fat_sector = fat_start_sector + (fat_offset / bytes_per_sector); // 0x810 + (8/512) = 0x810
//     uint32_t ent_offset = fat_offset % bytes_per_sector;
//     uint16_t entry;
//     ata0m.Read28(fat_sector, sizeof(entry), (uint8_t*)&entry);
//     return entry;
// }

uint16_t Read_FAT_Entry(uint16_t cluster) 
{
    // Calculate the offset of the FAT entry
    uint32_t fat_offset = cluster * 2;
    uint32_t fat_sector = fat_start_sector + (fat_offset / bytes_per_sector);
    uint32_t ent_offset = fat_offset % bytes_per_sector;

    // Buffer to read the entire sector
    uint8_t buffer[bytes_per_sector];
    
    // Read the FAT sector
    ata0m.Read28(fat_sector, bytes_per_sector, buffer);

    // Extract the 16-bit FAT entry
    uint16_t entry = *(uint16_t*)&buffer[ent_offset];

    // Return the FAT entry
    return entry;
}

void Read_File(uint8_t* name) 
{
    // In: name of file to read
    // Iterate over existing file names using file_names function
    // If match is found, using the parsed cluster data and reading from data region using the Read_Cluster method
    
    DirectoryEntry entries[16];
    //printf((uint8_t*)"Inside read file \n",0);

    helper_entry_struct* filenames = file_names(); // Index zero of array of names - values returned are names of things in entries

    for (int i = 0; i < 16; i++)
    {
        if (strcmp(name, filenames[i].name) == 0)
        {
            //printf((uint8_t*)"Found file in lists \n",0);

                       
            uint16_t cluster = filenames[i].firstClusterLow; 
            uint32_t file_size = filenames[i].size; // Parsed data correctly in example cluster is 0x3 and size is 0x14 (20)
            uint8_t buffer[bytes_per_sector * sectors_per_cluster]; // Assume maximum file size of 8KB for simplicity
            for (int k = 0; k < bytes_per_sector * sectors_per_cluster; k++)
            {
                buffer[k] = ' ';
            }
            uint32_t bytes_read = 0;

            //printf((uint8_t*)"cluster number: \n",0);
            //printfHex16(cluster);

            //printf((uint8_t*)"file size: \n",0);
            //printfHex32(file_size);

            while (cluster < 0xFFF8 && bytes_read < file_size) 
            {
                Read_Cluster(cluster, &buffer[bytes_read]); 
                bytes_read += sectors_per_cluster * bytes_per_sector;
                cluster = Read_FAT_Entry(cluster);
            }
            //printf((uint8_t*)"exited \n",0);
            printf(buffer, 0); // Print file contents
            return;

        }
    }
}

// void Write_FAT_Entry(uint16_t cluster, uint16_t value) 
// {
//     // In: specific cluster, specific value (next cluster)
//     // Out: None
//     uint32_t fat_offset = cluster * 2;
//     uint32_t fat_sector = fat_start_sector + (fat_offset / bytes_per_sector);
//     uint32_t ent_offset = fat_offset % bytes_per_sector;
//     uint16_t entry;
//     ata0m.Read28(fat_sector, bytes_per_sector, (uint8_t*)&entry);
//     ((uint16_t*)&entry)[ent_offset / 2] = value;
//     ata0m.Write28(fat_sector, (uint8_t*)&entry, bytes_per_sector);
//     ata0m.Flush();
// }

void Write_FAT_Entry(uint16_t cluster, uint16_t value) 
{
    // Calculate the offset of the FAT entry
    uint32_t fat_offset = cluster * 2;
    uint32_t fat_sector = fat_start_sector + (fat_offset / bytes_per_sector);
    uint32_t ent_offset = fat_offset % bytes_per_sector;

    // Buffer to read the entire sector
    uint8_t buffer[bytes_per_sector];
    
    // Read the FAT sector
    ata0m.Read28(fat_sector, bytes_per_sector, buffer);

    // Update the 16-bit FAT entry in the buffer
    *(uint16_t*)&buffer[ent_offset] = value;

    // Write the modified sector back to the disk
    ata0m.Write28(fat_sector, buffer, bytes_per_sector);
    ata0m.Flush();
}


void Write_File(uint8_t* name, uint8_t* data, uint32_t data_size) 
{
    // Basic implementation function to create a file, given an existing input
    // In: 1) name of file to create 2) data 3) size of data
    // STEPS:
    // 1) Understand how many clusters are needed (for now basic implementation means only one cluster)
    // 2) Iterate FAT and find available clusters 
    // 3) Fill cluster chain
    // 4) Create root directory entry containing data about the file creation
    // 5) Write data to clusters in data region using Write_Cluster method
    
    // Step 1: Calculate the number of clusters needed
    uint16_t clusters_needed = (data_size + (bytes_per_sector * sectors_per_cluster - 1)) / (bytes_per_sector * sectors_per_cluster);
    //if (clusters_needed == 1) printf((uint8_t*)"yepyepyep \n",0);
    
    // Step 2: Find available clusters
    uint16_t clusters[clusters_needed];
    uint16_t found_clusters = 0;
    
    for (uint16_t cluster = 3; cluster < total_clusters + 2; cluster++) 
    {
        //printf((uint8_t*)"Cluster observed \n",0);
        //printfHex16(cluster);
        //printf((uint8_t*)"\n",0);
        if (Read_FAT_Entry(cluster) == 0x0000) 
        {
            //printf((uint8_t*)"found \n",0);
            clusters[found_clusters] = cluster;
            found_clusters++;
            if (found_clusters == clusters_needed) 
            {
                break;
            }
        }
        else
        {
            //printf((uint8_t*)"This cluster is not abailable \n",0);

        }
    }

    for (int k = 0; k < clusters_needed; k++)
    {
        //printfHex16(clusters[k]);
        //printf((uint8_t*)"\n",0);
    }
    
    if (found_clusters < clusters_needed) {
        // Not enough space
        printf((uint8_t*)"Not enough space to write the file\n", 0);
        return;
    }
    
    // Step 3: Update the FAT entries
    for (uint16_t i = 0; i < clusters_needed - 1; i++) {
        // Point current cluster to the next cluster
        Write_FAT_Entry(clusters[i], clusters[i + 1]);
    }
    // Mark the last cluster as end of file
    Write_FAT_Entry(clusters[clusters_needed - 1], 0xFFFF);
    
    // Step 4: Write data to clusters
    uint32_t bytes_written = 0;
    for (uint16_t i = 0; i < clusters_needed; i++) 
    {
        Write_Cluster(clusters[i], &data[bytes_written]);
        bytes_written += sectors_per_cluster * bytes_per_sector;
    }
    
    // Step 5: Create directory entry
    DirectoryEntry new_entry;
    memset(&new_entry, 0, sizeof(DirectoryEntry));
    int i;
    for (i = 0; i < 8 && name[i] != '.' && name[i] != '\0'; i++) {
        new_entry.name[i] = name[i];
    }
    for (int j = 0; j < 3 && name[i] != '\0'; i++, j++) {
        new_entry.extension[j] = name[i + 1];
    }
    
    new_entry.firstClusterLow = clusters[0];
    new_entry.fileSize = data_size;

    // Read the root directory
    DirectoryEntry entries[16]; // Assuming 16 entries per sector
    ata0m.Read28(root_dir_start_sector, sizeof(entries), (uint8_t*)&entries);

    // Find an empty slot in the root directory
    for (i = 0; i < 16; i++) 
    {
        if (entries[i].name[0] == 0x00 || entries[i].name[0] == 0xE5) 
        {
            entries[i] = new_entry;
            break;
        }
    }
    
    // Write back the updated root directory
    ata0m.Write28(root_dir_start_sector, (uint8_t*)&entries, sizeof(entries));
    ata0m.Flush();
}