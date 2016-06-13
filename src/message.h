#ifndef JRPC_MESSAGE_H_
#define JRPC_MESSAGE_H_

Json::Value jrpc_init_message(int id);
int jrpc_message_send(int sockfd, Json::Value jobj);
Json::Value jrpc_read_message(int sockfd);
Json::Value jrpc_new_request(int id, const char *method);

#endif //JRPC_MESSAGE_H_