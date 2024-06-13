# Reconnect the loop device
loop_device=$(sudo losetup -fP --show os-disk.raw)

# Create a new directory for mounting the FAT16 partition
mount_point="/mnt/new-os-disk"
sudo mkdir -p ${mount_point}

# Mount the FAT16 partition to the new directory
sudo mount ${loop_device}p1 ${mount_point}

# View the files in the partition
cd ${mount_point}
ls -l

# Optional: View the content of a specific file
cat TESTFILE.txt

# Unmount the partition and disconnect the loop device
sudo umount ${mount_point}
sudo losetup -d ${loop_device}