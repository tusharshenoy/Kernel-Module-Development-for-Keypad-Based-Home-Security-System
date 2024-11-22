raspi-gpio set 5 pu
raspi-gpio set 6 pu
raspi-gpio set 27 pu
raspi-gpio set 22 pu

make clean

make all

sudo insmod securitysystem.ko

gcc -o app app.c

sudo mknod /dev/securitysystem c 236 0

sudo ./app

sudo rmmod securitysystem.ko

