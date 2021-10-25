#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/bcm.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "functions.h"

int main(int argc, char *argv[])
{
	struct sockaddr_can sa;
	struct message msg;

	int sock = socket(AF_CAN, SOCK_DGRAM, CAN_BCM);

	if (sock < 0) {
		perror("Socket failure");
		printf("Errno = %d\n", errno);
		exit(EXIT_FAILURE);
	}

	if (argc != 2) {
		printf("Please provide one argument\n");
		exit(EXIT_FAILURE);
	}

	char *ifname= argv[1];

	unsigned int getindex = if_nametoindex(ifname);

	if (getindex == 0) {
		perror("Interface is not valid");
		printf("Errno = %d\n", errno);
		exit(EXIT_FAILURE);
	}

	memset(&sa, 0, sizeof(sa));
	sa.can_family = AF_CAN;
	sa.can_ifindex = getindex;
	sa.can_addr.tp.rx_id = 0;
	sa.can_addr.tp.tx_id = 0;

	int con = connect(sock, (struct sockaddr *)&sa, sizeof(sa));

	if (con < 0) {
		perror("Connection failure");
		printf("Errno = %d\n", errno);
		exit(EXIT_FAILURE);
	}

	int r = txsetup(&msg, sock, &sa);
	r |= rxsetup(&msg, sock, &sa);
	r |= rxchanged(&msg, sock, &sa);
	return r;
}