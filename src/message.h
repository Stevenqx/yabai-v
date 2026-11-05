#ifndef MESSAGE_H
#define MESSAGE_H

void handle_message(FILE *rsp, char *message);
bool vspace_response_is_deferred(FILE *rsp);
bool message_loop_begin(char *socket_path);

#endif
