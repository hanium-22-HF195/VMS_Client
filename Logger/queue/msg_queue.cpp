#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "msg_queue.h"
#include "msg_type.h"

using namespace std;

// Logger(RECV) <- Web UI(SND)
void camera_cfg_recv(int &WIDTH, int &HEIGHT, int &FPS)
{
	int msgid;
	camera_cfg_msg_data data;
	int fps = FPS;
	unsigned char size, format;
	int width = WIDTH, height = HEIGHT;

	if (-1 == (msgid = msgget((key_t)CAMERA_CFG_MQ, IPC_CREAT | 0666)))
	{
		perror("    CAMERA_CFG_MQ creation failed");
		exit(0);
	}

	// datatype 0: receive all datatype
	// change datatype to 1 if you want to receive python data
	if (-1 == msgrcv(msgid, &data, sizeof(camera_cfg_msg_data) - sizeof(long), 0, IPC_NOWAIT))
	{
		cout << "    No Camera Configuaration value received." << endl;
	}
	else
	{
		if (data.data_type == TYPE_CAMERA_CFG)
		{
			//cout << "    Camera configuration received." << endl;
			memcpy(&size, data.data_buff, sizeof(unsigned char));
			memcpy(&format, data.data_buff + sizeof(unsigned char), sizeof(unsigned char));
			memcpy(&fps, data.data_buff + sizeof(unsigned char) * 2, sizeof(int));
		}

		// SET FPS
		FPS = fps;

		// SET SIZE
		switch (size)
		{
		case VGA:
			WIDTH = VGA_WIDTH;
			HEIGHT = VGA_HEIGHT;
			break;
		case HD:
			WIDTH = HD_WIDTH;
			HEIGHT = HD_HEIGHT;
			break;
		case CIF:
			WIDTH = CIF_WIDTH;
			HEIGHT = CIF_HEIGHT;
			break;
		}
		
		sleep(0.2);
		//camera_cfg_res_send();
	}
	cout << "    Camera Setting Changes to: " << endl;
	cout << "    width: " << WIDTH << "| height: " << HEIGHT << "| fps: " << FPS << endl;
}
/*
// Logger(RES) -> WEB UI(RECV)
void camera_cfg_res_send()
{
	int msgid;
	camera_cfg_res_msg_data data;
	unsigned char camera_cfg_res = CAMERA_CFG_RES;

	if (-1 == (msgid = msgget((key_t)CAMERA_CFG_RES_MQ, IPC_CREAT | 0666)))
	{
		perror("    CAMERA_CFG_RES_MQ creation failed");
		exit(0);
	}

	// by Header
	data.data_type = TYPE_CAMERA_CFG_RES;
	memcpy(data.data_buff, &camera_cfg_res, sizeof(unsigned char));

	if (-1 == msgsnd(msgid, &data, sizeof(camera_cfg_res_msg_data) - sizeof(long), 0))
	{
		perror("    response send failed ");
		exit(1);
	}

	else
	{
		cout << "    Camera_cfg_res sent." << endl;
	}
}  

// Logger(RECV) <- Web UI(REQ)
int Image_Hash_request(){

	int msgid;
	Image_hash_req_msg_data data;
	unsigned char image_hash_req;

	if (-1 == (msgid = msgget((key_t)IMAGE_HASH_REQ_MQ, IPC_CREAT | 0666)))
    {
        perror("    IMAGE_HASH_REQ_MQ creation failed.");
        exit(0);
    }

	if (-1 == msgrcv(msgid, &data, sizeof(Image_hash_req_msg_data) - sizeof(long), 0, IPC_NOWAIT))
    {
		cout << "    No request from Web UI.";
		return 0;
    }
	else {
		if (data.data_type == TYPE_IMAGE_HASH_REQ) {	
			cout << "    Web Requested Hash." << endl;
			cout << "    Image and Hash file saved. " << endl; 
			return 1;		
		}
	}
}

// Logger(RECV) ->  Web UI(REQ)
int Image_Hash_response() {

	int msgid;
	Image_hash_res_msg_data data;
	unsigned char res = IMAGE_HASH_RES;

	if (-1 == (msgid = msgget((key_t)IMAGE_HASH_RES_MQ, IPC_CREAT | 0666)))
	{
		perror("msgget() failed");
		exit(1);
	}

	// by Header
	data.data_type = TYPE_IMAGE_HASH_RES;
	memcpy(data.data_buff, &res, sizeof(unsigned char));

	if (-1 == msgsnd(msgid, &data, sizeof(Image_hash_res_msg_data) - sizeof(long), 0))
	{
		perror("    response send failed ");
		exit(1);
	}

	else
	{
		cout << "    Image Hash saved." << endl;
	}
}*/