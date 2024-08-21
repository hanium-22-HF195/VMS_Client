#include <iostream>
#include "config/Config.h"
#include "sign/sign.h"
#include "VMS_Client/VMS_Client.h"
#include "queue/msg_queue.h"
#include "media/Media.h"
#include "MK_Tree/MK_Tree.h"

using namespace std;

int main() {
    
    Config config;

    cout << "Width: " << config.getWidth() << ", Height: " << config.getHeight() << ", FPS: " << config.getFps() << endl;
    cout << "Original file path: " << config.getOrifilePath() << endl;
    cout << "Y frame file path: " << config.getYfilePath() << endl;
    cout << "Hash file path: " << config.getHashfilePath() << endl;

    Sign sign(config.getSignedHashBufSize());
   
    VMS_Client client(config.getServerIp(), config.getServerPort());
    client.send_pubkey_to_server(sign.getPublicKey());

    Media media(config);

    if (media.init_camera() == -1)
    {
        return -1;
    }
    media.capture_frame();
    media.convert_frames2gray();
    media.edge_detection_BGR();

    MK_Tree mk_tree;
    mk_tree.make_hash(media.getFeatureVectorQueue());

    sign.sign_hash(mk_tree.getHashQueue());

    return 0;
}
