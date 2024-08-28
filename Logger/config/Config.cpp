#include "Config.h"
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
using namespace std;

Config_cls::Config_cls() {
    Read_Logger_cfg(); 
}

Config_cls::~Config_cls() {
    
}

void Config_cls::Read_Logger_cfg() {
    string configPath = "../Sys_cfg.json"; 

    ifstream json_dir(configPath);
    if (!json_dir.is_open()) {
        cerr << "Could not open configuration file" << endl;
        exit(-1);
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    Json::Value value;
    JSONCPP_STRING errs;

    bool ok = Json::parseFromStream(builder, json_dir, &value, &errs);
    if (!ok) {
        cerr << "Failed to parse configuration: " << errs << endl;
        exit(-1);
    }

    width = value["Logger"]["width"].asInt();
    height = value["Logger"]["height"].asInt();
    fps = value["Logger"]["fps"].asInt();
    frameCount = value["Logger"]["frame count"].asInt();
    signedHashBufSize = value["Logger"]["signed hash bufsize"].asInt();
    orifilePath = value["Logger"]["original file path"].asString();
    yfilePath = value["Logger"]["Y frame file path"].asString();
    hashfilePath = value["Logger"]["hash file path"].asString();
    serverIp = value["Logger"]["Server IP addr"].asString();
    serverPort = value["Logger"]["Server port"].asInt();
}

int Config_cls::getWidth() const { return width; }
int Config_cls::getHeight() const { return height; }
int Config_cls::getFps() const { return fps; }
int Config_cls::getFrameCount() const { return frameCount; }
int Config_cls::getSignedHashBufSize() const { return signedHashBufSize; }
string Config_cls::getOrifilePath() const { return orifilePath; }
string Config_cls::getYfilePath() const { return yfilePath; }
string Config_cls::getHashfilePath() const { return hashfilePath; }
string Config_cls::getServerIp() const { return serverIp; }
int Config_cls::getServerPort() const { return serverPort; }
