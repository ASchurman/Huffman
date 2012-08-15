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

using std::fstream;
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
        
        // Returns a heap-alloc'd fstream for the file
        // pointed to by path, or NULL if the open fails.
        // If input == true, opens the file in input mode,
        // else opens the file in output mode.
        fstream* openFile(const char* path, bool input)
        {
            fstream* fileptr = new fstream();
            fstream& file = *fileptr;
            
            if(input)
            {
                file.open(path, ios::in);
            }
            else
            {
                file.open(path, ios::out | ios::trunc);
            }
            
            if(file.good())
            {
                return fileptr;
            }
            else
            {
                delete fileptr;
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
                
                // before pushing a codeword into words,
                // codeword.sym is initialized, so childWord.sym doesn't matter.
                // To get the compiler to be quiet, let's just initialize to 0.
                childWord.sym = 0;
                
                getCodewords(words, *(root.children[0]), childWord);
                
                childWord.code++;
                getCodewords(words, *(root.children[1]), childWord);
            }
        }
        
        // populates counts with character counts and returns the total
        // number of characters
        unsigned int countChars(fstream& input, map<char, double>& counts)
        {
            char c;
            unsigned int total = 0;
            
            while(input.get(c))
            {
                counts[c]++;
                total++;
            }
            
            return total;
        }
        
        // returns the heap-alloc'd root of the Huffman tree
        node* constructTree(map<char, double>& counts, unsigned int total)
        {
            // first create a leaf node for each symbol and add it to a
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
            
            return root;
        }
        
        // turns any Huffman code into a canonical one.
        // returns a heap-alloc'd codebook for this new code
        map<char, codeword>* canonize(vector<codeword>& words)
        {
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
            
            return bookptr;
        }
    }
    
    // returns 0 if successful, 1 if inpath is invalid, 2 if outpath is invalid
    char encode(const char* inpath, const char* outpath)
    {
        // open inpath
        fstream* inputptr = openFile(inpath, true);
        if(!inputptr)
        {
            return 1; // inpath is invalid
        }
        fstream& input = *inputptr;
        
        // first count symbols
        map<char, double> counts;
        unsigned int total = countChars(input, counts);

        // then construct Huffman tree
        node* root = constructTree(counts, total);
        
        // construct codebook by traversing tree
        vector<codeword>* wordsptr = new vector<codeword>();
        vector<codeword>& words = *wordsptr;
        
        codeword initWord;
        initWord.code = initWord.bits = initWord.sym = 0;
        
        getCodewords(words, *root, initWord);
        delete root;
        
        // put this Huffman code in canonical form
        map<char, codeword>* bookptr = canonize(words);
        map<char, codeword>& book = *bookptr;
        delete wordsptr; // we no longer need the old codebook
        
        // test this function so far by printing codebook
        typedef map<char, codeword>::iterator it_char_code_type;
        for(it_char_code_type it = book.begin(); it != book.end(); it++)
        {
            char sym = it->first;
            char code = it->second.code;
            char bits = it->second.bits;
            
            printf("Sym: %c\nCode: %x\nBits: %d\n\n", sym, code, bits);
        }
        
        // clean up
        input.close();
        delete inputptr;
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
