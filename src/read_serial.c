#include<stdio.h>
#include<stdlib.h>

int main(int argc, const char *argv[])
{
	
	int usbdev; /* handle to FTDI device */
	char command[5]; /* buffer of data to send */
	char response[20]; /* receive buffer */

	//system("stty -F /dev/ttyUSB0 9600 cs8 -cstopb -parity -icanon min 1 time 1");

	usbdev = open("/dev/ttyUSB0", O_RDWR);

	write(usbdev, command, 2);
	read(usbdev, response, 1);
	close(usbdev);

	return 0;
}


