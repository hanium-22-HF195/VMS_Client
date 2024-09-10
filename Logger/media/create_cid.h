#ifndef CREATE_CID_H
#define CREATE_CID_H

#include <string>

std::string getCID();
int init_shared_mem(int sequence);
uint64_t getIFMAC(const std::string &ifname);
int update_sequence(int shmid);

#endif
