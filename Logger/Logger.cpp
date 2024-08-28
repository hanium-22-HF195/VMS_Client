#include <iostream>
#include "config/Config.h"
#include "sign/sign.h"
#include "VMS_Client/VMS_Client.h"
#include "queue/msg_queue.h"
#include "media/Media.h"
#include "MK_Tree/MK_Tree.h"

using namespace std;

int main() {
    Config_cls config_inst;

    cout << "Width: " << config_inst.getWidth() << ", Height: " << config_inst.getHeight() << ", FPS: " << config_inst.getFps() << endl;
    cout << "Original file path: " << config_inst.getOrifilePath() << endl;
    cout << "Y frame file path: " << config_inst.getYfilePath() << endl;
    cout << "Hash file path: " << config_inst.getHashfilePath() << endl;

    Sign_cls sign_inst(config_inst.getSignedHashBufSize());
    VMS_Client_cls client_inst(config_inst.getServerIp(), config_inst.getServerPort());
    client_inst.send_pubkey_to_server(sign_inst.getPublicKey());

    Media_cls media_inst(config_inst);
    MK_Tree_cls mk_tree_inst;

    while (true) {
        if (media_inst.init_camera() == -1) {
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

    return 0;
}
