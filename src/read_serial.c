#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>

int main(int argc, const char *argv[])
{

	int usbdev; /* handle to FTDI device */
	char command[5]; /* buffer of data to send */
	char response[20]; /* receive buffer */
	//system("stty -F /dev/ttyUSB0 115200 cs8 -cstopb -parity -icanon min 1 time 1");

	if ( usbdev = open("/dev/ttyUSB0", O_RDWR) < 0) {
		perror("Unable to open device");
		exit(-1);
	}

	//write(usbdev, command, 2);
	read(usbdev, response, 1);
	close(usbdev);	

	printf("%s\n",response);

	return 0;
}
