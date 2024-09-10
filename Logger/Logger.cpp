#include <iostream>
#include "config/Config.h"
#include "sign/sign.h"
#include "VMS_Client/VMS_Client.h"
#include "queue/msg_queue.h"
#include "media/Media.h"
#include "MK_Tree/MK_Tree.h"

using namespace std;

void process_loop(Media_cls& media_inst, MK_Tree_cls& mk_tree_inst, Sign_cls& sign_inst, VMS_Client_cls& client_inst) {
    while (true) {
        if (media_inst.init_frame() == -1) {
            break;
        }
        media_inst.capture_frame();
        media_inst.convert_frames2gray();
        media_inst.edge_detection_BGR();

        mk_tree_inst.make_hash(media_inst.getFeatureVectorQueue());
        sign_inst.sign_hash(mk_tree_inst.getHashQueue());

        client_inst.send_image(media_inst.getCIDQueue(), mk_tree_inst.getHashQueue(), sign_inst.getSignHashQueue());

        media_inst.clearQueue();
        mk_tree_inst.clearQueue();
        sign_inst.clearQueue();
    }
}


int main() {

    Config_cls config_inst;

    //--------------------config test code---------------------------------------------------
    cout << "Width: " << config_inst.getWidth() << ", Height: " << config_inst.getHeight();
    cout << ", FPS: " << config_inst.getFps() << endl;
    cout << "Original file path: " << config_inst.getOrifilePath() << endl;
    cout << "Y frame file path: " << config_inst.getYfilePath() << endl;
    cout << "Hash file path: " << config_inst.getHashfilePath() << endl;


    Sign_cls sign_inst(config_inst.getSignedHashBufSize());

    VMS_Client_cls client_inst(config_inst.getServerIp(), config_inst.getServerPort());
    client_inst.send_pubkey_to_server(sign_inst.getPublicKey());

    Media_cls media_inst(config_inst);

    MK_Tree_cls mk_tree_inst;

    //----------camera config---------------
    int width = config_inst.getWidth();
    int height = config_inst.getHeight();
    int fps = config_inst.getFps();
    camera_cfg_recv(width, height, fps);

    process_loop(media_inst, mk_tree_inst, sign_inst, client_inst);

    return 0;
}
