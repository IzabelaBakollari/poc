#ifndef TEST_H
#define TEST_H

struct message {
	struct bcm_msg_head b;
	struct canfd_frame f;
};

extern void print_message(struct message *msg, int s);

extern int receive_and_check(struct message *msg, int sock, struct sockaddr_can *sa);

extern void txsetup(struct message *msg);

extern void rxsetup(struct message *msg);

extern void rxchanged(struct message *msg);

#endif
