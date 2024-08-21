#ifndef NODE_H
#define NODE_H

#include <string>

struct MerkleNode {
    std::string hash;
    MerkleNode *left;
    MerkleNode *right;

    MerkleNode(std::string data);
};

#endif /* NODE_H */