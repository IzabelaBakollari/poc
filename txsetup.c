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

void txsetup(struct message *msg)
{
	memset(msg, 0, sizeof(*msg));

	msg->b.opcode = TX_SETUP;
	msg->b.flags = CAN_FD_FRAME | SETTIMER | STARTTIMER | TX_COUNTEVT;
	msg->b.count = 2;
	msg->b.ival1.tv_sec = msg->b.ival2.tv_sec = 1;
	msg->b.ival1.tv_usec = msg->b.ival2.tv_usec = 1;
	msg->b.can_id = 0;
	msg->b.nframes = 1;
}
