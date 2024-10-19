#ifndef CONFIG_H
#define CONFIG_H

#include <string>
using namespace std;

class Config_cls {
private:
    int width;
    int height;
    int fps;
    int frameCount;
    int signedHashBufSize;
    string orifilePath;
    //string yfilePath;
    //string hashfilePath;
    string objectDetectorIp;
    int objectDetectorPort;
    string serverIp;
    int serverPort;

public:
    Config_cls();  
    ~Config_cls();  

    void Read_Logger_cfg(); 

    int getWidth() const;
    int getHeight() const;
    int getFps() const;
    int getFrameCount() const;
    int getSignedHashBufSize() const;
    string getOrifilePath() const;
    //string getYfilePath() const;
    //string getHashfilePath() const;
    string getObjectDetectorIp() const;
    int getObjectDetectorPort() const;
    string getServerIp() const;
    int getServerPort() const;
};

#endif // CONFIG_H
