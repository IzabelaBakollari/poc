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

int rxchanged(struct message *msg, int sock, struct sockaddr_can *sa)
{
	memset(msg, 0, sizeof(*msg));

	msg->b.opcode = TX_SEND;
	msg->b.flags = CAN_FD_FRAME;
	msg->b.count = 0;
	msg->b.ival1.tv_sec = msg->b.ival2.tv_sec = 0;
	msg->b.ival1.tv_usec = msg->b.ival2.tv_usec = 1;
	msg->b.can_id = 0;
	msg->b.nframes = 1;

	return receive_and_check(msg, sock, sa);
}
