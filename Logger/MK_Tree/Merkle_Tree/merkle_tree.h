#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include "node.h"
#include "picosha2.h"
#include "misc.h"
#include <vector>
#include <string>

struct MerkleTree {
    MerkleNode* root;
    MerkleTree(std::vector<MerkleNode*> blocks);
    ~MerkleTree();
    void printTree(MerkleNode *n, int indent);
    void deleteTree(MerkleNode *n);
};


#endif /* MERKLE_TREE_H */