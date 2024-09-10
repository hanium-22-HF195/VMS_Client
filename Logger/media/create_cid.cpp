#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <cstdint>
#include <fstream>
#include <streambuf>
#include <regex>
#include <sstream>
#include <string>


using namespace std;

#define SHMSZ     27


int init_shared_mem() {
    int shmid;

    /*
     * Create the segment.
     */
    key_t key = ftok("shmfile", 65);
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    char *str = (char *)shmat(shmid, (void *)0, 0); // shmat to attach to shared memory
    if (str[0] == '\0') {  // 메모리가 비어있으면 조건실행
        std::ostringstream s;
        s << 100;  // 초기 value
        strcpy(str, s.str().c_str());
    }
    shmdt(str); // to detach from the shared memory

    return shmid;
}

uint64_t getIFMAC(const string &ifname) {
 ifstream iface("/sys/class/net/" + ifname + "/address");
  string str((istreambuf_iterator<char>(iface)), istreambuf_iterator<char>());
  if (str.length() > 0) {
    string hex = regex_replace(str, std::regex(":"), "");
    return stoull(hex, 0, 16);
  } else {
    return 0;
  }
} 

int update_sequence (int shmid) {
    
    // read a sequence number in share memory
    char *str = (char*) shmat(shmid,(void*)0,0); // to attach the process to shared memory
    printf("Data read from memory: %s\n",str);
    int sequence = atoi(str);
    int current_sequence = sequence;
    shmdt(str);
    
    // update a sequence number in share memory
    char *str1 = (char *)shmat(shmid, (void *)0, 0); // shmat to attach to shared memory
    std::ostringstream s;
    s << (sequence + 1);
    strcpy(str, s.str().c_str());
    shmdt(str1); // to detach from the shared memory

    return current_sequence;
}

#include <string>

string getCID() {
    string iface = "eth0";
    int shmid;
    char buf[100];

    shmid = init_shared_mem();

    int current_sequence = update_sequence(shmid);
    sprintf(buf, "%012llX-%4d", getIFMAC(iface), current_sequence);

    //shmctl(shmid, IPC_RMID, NULL); // to deallocate the shared memory

    return string(buf); 
}