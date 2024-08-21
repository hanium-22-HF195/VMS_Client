#ifndef CONFIG_H
#define CONFIG_H

#include <string>
using namespace std;

class Config {
private:
    int width;
    int height;
    int fps;
    int frameCount;
    int signedHashBufSize;
    string orifilePath;
    string yfilePath;
    string hashfilePath;
    string serverIp;
    int serverPort;

public:
    Config();  
    ~Config();  

    void Read_Logger_cfg(); 

    int getWidth() const;
    int getHeight() const;
    int getFps() const;
    int getFrameCount() const;
    int getSignedHashBufSize() const;
    string getOrifilePath() const;
    string getYfilePath() const;
    string getHashfilePath() const;
    string getServerIp() const;
    int getServerPort() const;
};

#endif // CONFIG_H
