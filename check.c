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


int receive_and_check(struct message *msg, int sock, struct sockaddr_can *sa)
{
	int s = sendto(sock, msg, sizeof(*msg), 0, (struct sockaddr *)sa,
			sizeof(*sa));

	if (s < 0) {
		perror("Sending incomplete message");
		printf("Errno = %d\n", errno);
		exit (EXIT_FAILURE);
	}

	print_message(msg, s);

	socklen_t len = 0;

	s = recvfrom(sock, msg, sizeof(*msg), 0,
			(struct sockaddr *)sa, &len);

	if (s < sizeof(msg->b)) {
		printf("Recieved message is incomplete\n");
		exit(EXIT_FAILURE);
	}

	print_message(msg, s);


	for (int i = 12; i < 16; i++) {
		char n = ((unsigned char*) msg)[i];
		if (n != 0) {
			printf("Padding bytes with index number %x are corrupted\n", i);
			printf("%x\n ", n);
			return 1;
		}
	}
	return 0;
}

void print_message(struct message *msg, int s)
{
	for (int i=0; i<s; i++)
		printf("%x ", ((unsigned char*) msg)[i]);
	printf("\n");
}
