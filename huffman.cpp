/* 
 * File:   huffman.cpp
 * Author: Alexander Schurman, alexander.schurman@gmail.com
 *
 * Created on August 13, 2012
 */

#include <fstream>
#include <map>
#include <queue>
#include <vector>
#include <stdio.h>

#include "huffman.h"
#include "node.h"

using std::ifstream;
using std::ofstream;
using std::ios;
using std::map;
using std::priority_queue;

namespace huffman
{
    namespace
    {
        ifstream* openInput(const char* path, bool binary)
        {
            ifstream* inputptr = new ifstream();
            ifstream& input = *inputptr;
            
            if(binary)
            {
                input.open(path, ios::in | ios::binary);
            }
            else
            {
                input.open(path, ios::in);
            }
            
            if(input.good())
            {
                return inputptr;
            }
            else
            {
                delete inputptr;
                return NULL;
            }
        }
        
        ofstream* openOutput(const char* path, bool binary)
        {
            ofstream* outputptr = new ofstream();
            ofstream& output = *outputptr;
            
            if(binary)
            {
                output.open(path, ios::trunc | ios::out | ios::binary);
            }
            else
            {
                output.open(path, ios::trunc | ios::out);
            }
            
            if(output.good())
            {
                return outputptr;
            }
            else
            {
                delete outputptr;
                return NULL;
            }
        }
        
        
        struct codeword
        {
            char code;
            char bits; // number of bits in char code
        };
        
        // populates map<char,codeword> words with symbol-code pairs
        // produced by traversing the tree rooted with given root
        void getCodewords(map<char, codeword>& words,
                          const node& root,
                          codeword currWord)
        {
            if(root.children[0] == NULL && root.children[1] == NULL)
            {
                // we're at a leaf!
                words[root.sym] = currWord;
                return;
            }
            else
            {
                // neither child is NULL, since an internal node MUST be
                // constructed with 2 children
                
                codeword childWord;
                childWord.bits = currWord.bits + 1;
                childWord.code = currWord.code << 1;
                
                getCodewords(words, *(root.children[0]), childWord);
                
                childWord.code++;
                getCodewords(words, *(root.children[1]), childWord);
            }
        }
        
        class freqCompare
        {
        public:
            bool operator() (const node* a, const node* b)
            {
                return a->freq > b->freq;
            }
        };
    }
    
    // returns 0 if successful, 1 if inpath is invalid, 2 if outpath is invalid
    char encode(const char* inpath, const char* outpath)
    {
        // open inpath in normal mode; not binary
        ifstream* inputptr = openInput(inpath, false);
        if(!inputptr)
        {
            return 1; // inpath is invalid
        }
        ifstream& input = *inputptr;
        
        // first count symbols
        char c;
        unsigned int total = 0;
        map<char, double> counts;

        while(input.get(c))
        {
            counts[c]++;
            total++;
        }
        input.close();
        delete inputptr;

        // now create a leaf node for each symbol and add it to a
        // priority queue
        priority_queue<node*, std::vector<node*>, freqCompare> pq;
        
        typedef map<char, double>::iterator it_char_doub_type;
        for(it_char_doub_type it = counts.begin(); it != counts.end(); it++)
        {
            double freq = it->second / total;
            
            node* newnode = new node(freq, it->first);
            
            pq.push(newnode);
        }

        // pop nodes from queue to construct Huffman tree
        while(pq.size() > 1)
        {
            node* a = pq.top();
            pq.pop();
            node* b = pq.top();
            pq.pop();
            
            node* internalNode = new node(a, b);
            pq.push(internalNode);
        }
        node* root = pq.top();
        
        // construct codebook by traversing tree
        map<char, codeword>* wordsptr = new map<char, codeword>();
        map<char, codeword>& words = *wordsptr;
        
        codeword initWord;
        initWord.code = initWord.bits = 0;
        
        getCodewords(words, *root, initWord);
        delete root;
        
        // test this function so far by printing codebook
        typedef map<char, codeword>::iterator it_char_code_type;
        for(it_char_code_type it = words.begin(); it != words.end(); it++)
        {
            char sym = it->first;
            char code = it->second.code;
            char bits = it->second.bits;
            
            printf("Sym: %c\nCode: %x\nBits: %d\n\n", sym, code, bits);
        }
        
        return 0; // success
    }
    
    // returns 0 if successful, 1 if inpath is invalid, 2 if outpath is invalid
    char decode(const char* inpath, const char* outpath)
    {
        // open inpath in binary mode
//        ifstream* inputptr = openInput(inpath, true);
//        if(!inputptr)
//        {
//            return 1; // inpath is invalid;
//        }
//        ifstream& input = *inputptr;
//        
//        input.close();
//        delete inputptr;
        return 0; // success
    }
}
