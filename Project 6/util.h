//
// STARTER CODE: util.h
//
// TODO:  Write your own header and fill in all functions below.
//

#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <unordered_map>
#include <queue>          // std::priority_queue
#include <vector>         // std::vector
#include <functional>     // std::greater
#include <string>
#include "bitstream.h"
#include "util.h"
#include "hashmap.h"
#include "mymap.h"

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};

class prioritize {
  public:
    bool operator() (HuffmanNode* hn1, HuffmanNode* hn2) {
      return hn1->count > hn2->count;
    }
};
//  Recusion helper function to free the tree
void freeTreePO(HuffmanNode* curr) {
  if (curr == nullptr) {
    return;
  } else {
    freeTreePO(curr -> zero);
    freeTreePO(curr -> one);
    delete curr;
  }
}
//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
  HuffmanNode* curr = node;
  freeTreePO(curr);
}

//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmap &map) {
    if (isFile == true) {
      ifstream inFile(filename);
      char c;
      while (inFile.get(c)) {
        if (map.containsKey(c) != true) {
          map.put(c, 1);
        } else {
          map.put(c, map.get(c) + 1);
        }
      }
    } else {
      for (auto c : filename) {
        if (map.containsKey(c) != true) {
          map.put(c, 1);
        } else {
          map.put(c, map.get(c) + 1);
        }
      }
    }
    map.put(PSEUDO_EOF, 1);
}

//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmap &map) {
    priority_queue <HuffmanNode*, vector<HuffmanNode*>, prioritize> pq;
    vector <int> vectorKeys;
    vectorKeys = map.keys();  // Create all vector contains all keys
    for (int i = 0; i < map.size(); i++) {
      // create huffnode
      HuffmanNode* newNode = new HuffmanNode();
      newNode -> character = vectorKeys.at(i);
      newNode -> count = map.get(vectorKeys.at(i));
      newNode -> zero = nullptr;
      newNode -> one = nullptr;
      // insert into pq
      pq.push(newNode);
    }
    // conversion to a tree
    while (pq.size() > 1) {
      // 1. grab 2 nodes from the queue;
      HuffmanNode* left = pq.top();
      pq.pop();
      HuffmanNode* right = pq.top();
      pq.pop();
      // 2. create a new node where frequency is the sum
      HuffmanNode* newNode = new HuffmanNode();
      newNode -> character = NOT_A_CHAR;
      newNode -> count = (left -> count) + (right -> count);
      // 3. link up
      newNode -> zero = left;
      newNode -> one = right;
      // 4. Enqueue the node back to the queue
      pq.push(newNode);
    }
    return pq.top();
}
// This is a inorder traversal to builg the string
void building_Map(mymap <int, string> &encodingMap, 
HuffmanNode* curr, string path) {
    if (curr == nullptr) {
      return;
    } else {
      // if != Not a char, that means its a leaf node. insert into mymapmap
      if (curr -> character != NOT_A_CHAR) {
        encodingMap.put(curr -> character, path);
      }
      path += "0";
      building_Map(encodingMap, curr -> zero, path);
      path.pop_back();
      path += "1";
      building_Map(encodingMap, curr -> one, path);
    }
}
//
// *This function builds the encoding map from an encoding tree.
//
mymap <int, string> buildEncodingMap(HuffmanNode* tree) {
    mymap <int, string> encodingMap;
    HuffmanNode* curr = tree;
    string path = "";
    building_Map(encodingMap, curr, path);
    
    return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, mymap <int, string> &encodingMap,
              ofbitstream& output, int &size, bool makeFile) {
  string finalStr = "";
  char c;
  // reading the char to determine what to add to the string
  while (input.get(c)) {
    finalStr += encodingMap.get((int)c);  
  }
  finalStr += encodingMap.get(PSEUDO_EOF);
  if (makeFile == true) {
      for (unsigned int i = 0; i < finalStr.size(); i++) {
        output.writeBit(finalStr.at(i) - 48);
      }
  }
  size = finalStr.size();
  return finalStr;
}

//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    HuffmanNode* curr = encodingTree;
    string result = "";
    while (!input.eof()) {
      int bit = input.readBit();
      if (bit == 1) {
        curr = curr -> one;
        if (curr -> character != NOT_A_CHAR) {
          if (curr -> character == PSEUDO_EOF) {
            break;
          }
          result += curr -> character;
          curr = encodingTree;
        }
      } else if (bit == 0){
        curr = curr -> zero;
        if (curr -> character != NOT_A_CHAR) {
          if (curr -> character == PSEUDO_EOF) {
            break;
          }
          result += curr -> character;
          curr = encodingTree;
        }
      }
    }
    output << result;
    return result;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
  hashmap FrequencyMap;
  ifstream input(filename);
  mymap <int, string> EncodingMap;
  // create filename.txt.huf
  string huf = "";
  huf += filename;
  huf += ".huf";
  ofbitstream output(huf);
  int size = 0;
  string result;

  // building FrequencyMap
  buildFrequencyMap(filename, true, FrequencyMap);
  // building encoding tree
  HuffmanNode* EncodedTree = buildEncodingTree(FrequencyMap);
  // building encoding map;
  EncodingMap = buildEncodingMap(EncodedTree);
  // encoding the file
  output << FrequencyMap;
  result = encode(input, EncodingMap, output, size, true);

  // freeing the EncodedTree
  freeTree(EncodedTree);
  return result;
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {
  hashmap FrequencyMap;
  ifbitstream input (filename);
  // creating a filename_unc.txt
  string unc = filename;
  for (int i = 0; i < 8; i++) {
    unc.pop_back();
  }
  unc += "_unc.txt";
  ofstream output(unc);
  string result;

  // building FrequencyMap
  input >> FrequencyMap;
  // building encoding tree
  HuffmanNode* EncodedTree = buildEncodingTree(FrequencyMap);
  // dencoding the file
  result = decode(input, EncodedTree, output);
  // freeing the tree
  freeTree(EncodedTree);
  return result;
}
