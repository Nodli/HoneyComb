#include "KDTree.hpp"

#include <deque>
#include <algorithm>
#include <functional>

#include <iostream>

bool KDNode::isLeaf(){
    return !(children[0] || children[1]);
}

void KDTree::build(std::vector<Star>::iterator begin, std::vector<Star>::iterator end){

    std::function<bool(const Star* const sA, const Star* const sB)> compareDim[3] = {
        [](const Star* const sA, const Star* const sB) -> bool{
            return (sA->mPosition.mX < sB->mPosition.mX);},
        [](const Star* const sA, const Star* const sB) -> bool{
            return (sA->mPosition.mY < sB->mPosition.mY);},
        [](const Star* const sA, const Star* const sB) -> bool{
            return (sA->mPosition.mZ < sB->mPosition.mZ);}
        };

    // creating the root node
    nodes.reserve(std::distance(begin, end)); // most likely too much but avoids reallocations
    nodes.resize(1);
    root = &nodes[0];

    root->temp.reserve(std::distance(begin, end));
    for(;
        begin != end;
        ++begin){

        root->temp.push_back(&(*begin));
        ++begin;
    }

    // initializing the queue of nodes to build
    std::deque<std::pair<unsigned int, KDNode*>> toBuild; // level, node
    toBuild.emplace_back(0, root);

    // building the tree
    while(!toBuild.empty()){
        KDNode* currentNode = toBuild.front().second;

        if(currentNode->temp.size() > 2){

            // finding the split point
            unsigned int medianIndex = currentNode->temp.size() / 2;
            std::nth_element(currentNode->temp.begin(),
                    currentNode->temp.begin() + medianIndex,
                    currentNode->temp.end(),
                    compareDim[toBuild.front().first % 3]);

            // saving the split point
            currentNode->star = *(currentNode->temp.begin() + medianIndex);

            // creating the child nodes
            unsigned int offset = nodes.size();
            nodes.resize(offset + 2);

            currentNode->children[0] = &nodes[offset];
            currentNode->children[1] = &nodes[offset + 1];

            nodes[offset].temp.insert(nodes[offset].temp.begin(),
                    currentNode->temp.begin(),
                    currentNode->temp.begin() + medianIndex);
            nodes[offset + 1].temp.insert(nodes[offset + 1].temp.begin(),
                    currentNode->temp.begin() + (medianIndex + 1),
                    currentNode->temp.end());

            // enqueue the child nodes for build if they are not leaves
            toBuild.emplace_back(toBuild.front().first + 1, currentNode->children[0]);
            toBuild.emplace_back(toBuild.front().first + 1, currentNode->children[1]);

        // one child node only
        }else if(currentNode->temp.size() == 2){

            // allocating the nodes
            unsigned int offset = nodes.size();
            nodes.resize(offset + 1);
            currentNode->children[0] = &nodes[offset];

            // attributing a star to each node
            if(compareDim[toBuild.front().first % 3](currentNode->temp[0], currentNode->temp[1])){
                currentNode->star = currentNode->temp[1];
                currentNode->children[0]->star = currentNode->temp[0];
            }else{
                currentNode->star = currentNode->temp[0];
                currentNode->children[0]->star = currentNode->temp[1];
            }

        // leaf node ie just put the pointer from temp into star
        }else{

            currentNode->star = currentNode->temp[0];
            currentNode->temp.clear();
        }

        currentNode->temp.clear();
        toBuild.pop_front();

    }
}

Star* KDTree::searchNN(const Star* const star){

    auto sqDist = [](const Star* sA, const Star* sB) -> float{
        return (sB->mPosition.mX - sA->mPosition.mX) * (sB->mPosition.mX - sA->mPosition.mX)
            + (sB->mPosition.mY - sA->mPosition.mY) * (sB->mPosition.mY - sA->mPosition.mY)
            + (sB->mPosition.mZ - sA->mPosition.mZ) * (sB->mPosition.mZ - sA->mPosition.mZ);
    };


    Star* best = root->star;
    float bestDist = sqDist(root->star, star);

    std::function<void(KDNode*, const unsigned int)> searchStep =
        [this, star, &best, &bestDist, &sqDist, &searchStep]
        (KDNode* node, const unsigned int depth){

        if(node->isLeaf()){

            float currentDist = sqDist(star, node->star);

            if(currentDist < bestDist){
                best = node->star;
                bestDist = currentDist;
            }

        }else{
            const unsigned int dim = depth % 3;

            if(star->mPosition.mDim[dim] < node->star->mPosition.mDim[dim]){

                // search left first
                if(node->children[0]){
                    searchStep(node->children[0], depth + 1);
                }

                // keep searching right if the search radius still reaches the right split
                if(star->mPosition.mDim[dim] + bestDist >= node->star->mPosition.mDim[dim]
                    && node->children[1]){
                    searchStep(node->children[1], depth + 1);
                }

            }else{

                // search right first
                if(node->children[1]){
                    searchStep(node->children[1], depth + 1);
                }

                // keep searching left if the search radius still reaches the left split
                if(star->mPosition.mDim[dim] - bestDist <= node->star->mPosition.mDim[dim]
                    && node->children[0]){
                    searchStep(node->children[0], depth + 1);
                }
            }
        }
    };

    searchStep(root, 0);

    return best;
}

void KDTree::clear(){
    root = nullptr;
    nodes.clear();
}
