1. To build
make 
 
2. Install kernel module
sudo insmod main.ko

3. Check kernel message
sudo dmesg | tail -10

4. Check device file
ls -l /dev/my_device

5. Write data to the device
printf "HELLO" | sudo tee /dev/my_device > /dev/null

6. Read data from the device
sudo dd if=/dev/my_device bs=1 count=5 status=none

7. Unload the module
sudo rmmod main.ko