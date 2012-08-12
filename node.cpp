/* 
 * File:   Node.cpp
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 * 
 * Created on August 11, 2012
 */

#include "node.h"
#include <cstddef>

// internal node constructor
node::node(double frequency, node *leftChild, node *rightChild)
{
    freq = frequency;
    
    children = new node[2];
    children[0] = leftChild;
    children[1] = rightChild;
    
    parent = NULL;
    
    // internal nodes don't represent a single symbol
    sym = 0;
}

// leaf node constructor
node::node(double frequency, char symbol)
{
    freq = frequency;
    
    // these children will ALWAYS be NULL since this is a leaf node
    children = new node[2];
    children[0] = NULL;
    children[1] = NULL;
    
    parent = NULL;
    
    sym = symbol;
}

node::node(const node& orig)
{
    freq = orig->freq;
    
    children = new node[2];
    children[0] = orig->children[0];
    children[1] = orig->children[1];
    
    parent = orig->parent;
    
    sym = orig->sym;
}

node::~node()
{
    // deletes the children as well as the array of pointers to them
    delete[] children;
}
