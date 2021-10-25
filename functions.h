#ifndef FUNCTIONS_H
#define FUNCTIONS_H

struct message {
	struct bcm_msg_head b;
	struct canfd_frame f;
};

extern int receive_and_check(struct message *msg, int sock, struct sockaddr_can *sa);

extern int txsetup(struct message *msg, int sock, struct sockaddr_can *sa);
 
extern int rxsetup(struct message *msg, int sock, struct sockaddr_can *sa);

extern int rxchanged(struct message *msg, int sock, struct sockaddr_can *sa );

#endif