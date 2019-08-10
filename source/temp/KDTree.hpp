#ifndef KDTREE_H
#define KDTREE_H

struct KDNode{
    bool isLeaf();

    Star* star;
    std::vector<Star*> temp;
    KDNode* children[2] = {nullptr, nullptr};
};

struct KDTree{
    void build(std::vector<Star>::iterator begin, std::vector<Star>::iterator end);
    Star* searchNN(const Star* const star);
    void clear();

    KDNode* root = nullptr;
    std::vector<KDNode> nodes;
};

#endif
