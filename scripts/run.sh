#!/bin/bash

# Bash script to 1) Create hard disk 2) execute code
DEPENDENCIES_DIR="./dependencies"
DISK_FILE="os-disk.raw"
DISK_PATH="$DEPENDENCIES_DIR/$DISK_FILE"

# Change directory to dependencies folder

# Check if os-disk.raw exists in the dependencies folder
if [ ! -e "dependancies/os-disk.raw" ]; then
    # If the file doesn't exist, create it
    /usr/bin/qemu-img create -f raw os-disk.raw 512M
    loop_device=$(sudo losetup -fP --show os-disk.raw) # connecting to /dev enables to treat the raw file as hard drive and add partitions and partitions tables
    sudo parted ${loop_device} --script mklabel msdos
    sudo parted ${loop_device} --script mkpart primary fat16 1MiB 100%
    sudo parted ${loop_device} --script set 1 boot on
    sudo mkfs.vfat -F 16 ${loop_device}p1
    # add a file for testing
    #----------------------------------------
    sudo mkdir -p /mnt/os-disk
    sudo mount ${loop_device}p1 /mnt/os-disk

    # Add a text file to the partition
    echo "This is a test file" | sudo tee /mnt/os-disk/TESTFILE.txt
    echo "This is a test file 2" | sudo tee /mnt/os-disk/TESTFILD.txt
    echo "This is a test file 3" | sudo tee /mnt/os-disk/TESTFILV.txt

    # Unmount the partition and disconnect the loop device
    sudo umount /mnt/os-disk
    #-------------------------------------------- # mounting and adding file. /mnt enables to treat the disk image as actual hard drive and add files and folders to it.
    #sudo dd if=/dev/loop0 bs=512 count=1 | hexdump -C
    sudo losetup -d ${loop_device} # disconnect mount
    sudo mv os-disk.raw dependancies/ 
    sudo cp dependancies/os-disk.raw test/test.raw
fi

# Run qemu-system-i386 with mykernel.iso and os-disk.qcow2
#sudo hexdump -C dependancies/os-disk.raw
/usr/bin/qemu-system-i386 -cdrom ./objects/mykernel.iso -drive file=dependancies/os-disk.raw,format=raw,cache=none -boot d -m 512 -no-reboot -no-shutdown -M smm=off -s

