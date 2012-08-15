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
#include <algorithm>

#include "huffman.h"
#include "node.h"

using std::ifstream;
using std::ofstream;
using std::ios;
using std::map;
using std::vector;
using std::priority_queue;

namespace huffman
{
    namespace
    {
        class freqCompare
        {
        public:
            bool operator() (const node* a, const node* b)
            {
                return a->freq > b->freq;
            }
        };
        
        struct codeword
        {
            char sym;
            char code;
            char bits; // number of bits in char code
        };
        
        bool codewordCompare(const codeword& a, const codeword& b)
        {
            if(a.bits < b.bits) // sort first by code length
            {
                return true;
            }
            else if(a.bits > b.bits)
            {
                return false;
            }
            else if(a.sym <= b.sym) // ...then by symbol
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        
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
        
        
        // populates map<char,codeword> words with symbol-code pairs
        // produced by traversing the tree rooted with given root
        void getCodewords(vector<codeword>& words,
                          const node& root,
                          codeword currWord)
        {
            if(root.children[0] == NULL && root.children[1] == NULL)
            {
                // we're at a leaf!
                currWord.sym = root.sym;
                words.push_back(currWord);
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
        priority_queue<node*, vector<node*>, freqCompare> pq;
        
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
        vector<codeword>* wordsptr = new vector<codeword>();
        vector<codeword>& words = *wordsptr;
        
        codeword initWord;
        initWord.code = initWord.bits = initWord.sym = 0;
        
        getCodewords(words, *root, initWord);
        delete root;
        
        // sort words by length and symbol to get a canonical Huffman codebook
        std::sort(words.begin(), words.end(), codewordCompare);
        
        map<char, codeword>* bookptr = new map<char, codeword>();
        map<char, codeword>& book = *bookptr;
        
        words[0].code = 0; // first word is zero
        book[words[0].sym] = words[0];
        for(unsigned int i = 1; i < words.size(); i++)
        {
            // each word is one greater than last
            words[i].code = words[i-1].code + 1;
            
            // if a word is longer than the previous one,
            // left shift until it's the appropriate length
            if(words[i].bits > words[i-1].bits)
            {
                for(int j = words[i-1].bits; j < words[i].bits; j++)
                {
                    words[i].code = words[i].code << 1;
                }
            }
            
            book[words[i].sym] = words[i];
        }     
        delete wordsptr;
        
        // test this function so far by printing codebook
        typedef map<char, codeword>::iterator it_char_code_type;
        for(it_char_code_type it = book.begin(); it != book.end(); it++)
        {
            char sym = it->first;
            char code = it->second.code;
            char bits = it->second.bits;
            
            printf("Sym: %c\nCode: %x\nBits: %d\n\n", sym, code, bits);
        }
        
        delete bookptr;
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
