/* 
 * File:   Node.h
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 *
 * Created on August 11, 2012
 * 
 * Represents a node in our Huffman tree
 */

#ifndef NODE_H
#define	NODE_H

class node
{
public:
    // internal node constructor.
    node(node* leftChild, node* rightChild);
    
    // leaf node constructor.
    node(double frequency, char symbol);
    
    // makes copy of orig
    node(const node& orig);
    
    virtual ~node();
    
    // the symbol contained in this node.
    // only used in leaf nodes.
    char sym;
    
    // The frequency of this node. Used in the priority queue.
    double freq;
    
    // Array of references to this node's children
    const node* children[2];
    
private:

};

#endif	/* NODE_H */

