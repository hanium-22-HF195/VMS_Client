void camera_cfg_res_send();                                  //Logger(SND)  -> Web UI(RECV)      B
void camera_cfg_recv(int &WIDTH, int &HEIGHT, int &FPS);     //Logger(RECV) <- Web UI(SND)       BBI

int Image_Hash_request();                                    //Logger(RECV) <- Web UI(REQ)       B
int Image_Hash_response();                                   //Logger(RECV) <- Web UI(SND))      B