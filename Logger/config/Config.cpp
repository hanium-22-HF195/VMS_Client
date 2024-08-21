#include "Config.h"
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
using namespace std;

Config::Config() {
    Read_Logger_cfg(); 
}

Config::~Config() {
    
}

void Config::Read_Logger_cfg() {
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

int Config::getWidth() const { return width; }
int Config::getHeight() const { return height; }
int Config::getFps() const { return fps; }
int Config::getFrameCount() const { return frameCount; }
int Config::getSignedHashBufSize() const { return signedHashBufSize; }
string Config::getOrifilePath() const { return orifilePath; }
string Config::getYfilePath() const { return yfilePath; }
string Config::getHashfilePath() const { return hashfilePath; }
string Config::getServerIp() const { return serverIp; }
int Config::getServerPort() const { return serverPort; }
