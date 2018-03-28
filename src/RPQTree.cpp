//
// Created by Nikolay Yakovets on 2018-02-02.
//

#include <iostream>
#include "RPQTree.h"

RPQTree::~RPQTree() {
    delete(left);
    delete(right);
}

RPQTree* RPQTree::strToTree(std::string &str) {

    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end()); // remove spaces

    int level = 0; // inside parentheses check

    // case /
    // most right '/' (but not inside '()') search and split
    for(int i=(int) str.size()-1;i>=0;--i){
        char c = str[i];
        if(c == ')'){
            ++level;
            continue;
        }
        if(c == '('){
            --level;
            continue;
        }
        if(level>0) continue;
        if(c == '/'){
            std::string left(str.substr(0,i));
            std::string right(str.substr(i+1));
            std::string payload(1, c);
            return new RPQTree(payload, strToTree(left), strToTree(right));
        }
    }

    if(str[0]=='('){
        //case ()
        //pull out inside and to strToTree
        for(int i=0;i<str.size();++i){
            if(str[i]=='('){
                ++level;
                continue;
            }
            if(str[i]==')'){
                --level;
                if(level==0){
                    std::string exp(str.substr(1, i-1));
                    return strToTree(exp);
                }
                continue;
            }
        }
    } else
        // case value
        return new RPQTree(str, nullptr, nullptr);

    std::cerr << "Error: parsing RPQ failed." << std::endl;
    return nullptr;
}

void RPQTree::print() {

    if(left == nullptr && right == nullptr) {
        std::cout << ' ' << data << ' ';
    } else {
        std::cout << '(' << data << ' ';
        if(left != nullptr) left->print();
        if(right!= nullptr) right->print();
        std::cout << ')';
    }

}

bool RPQTree::isConcat() {
    return (data == "/") && isBinary();
}

bool RPQTree::isBinary() {
    return left != nullptr && right != nullptr;
}

bool RPQTree::isUnary() {
    return left != nullptr && right == nullptr;
}

bool RPQTree::isLeaf() {
    return left == nullptr && right == nullptr;
}

std::vector<std::string> RPQTree::inOrderNodes(RPQTree *t) {
  auto v = std::vector<std::string>();

  if (t == nullptr) {
    return v;
  }

  auto left_v = RPQTree::inOrderNodes(t->left);
  v.insert(v.end(), left_v.begin(), left_v.end());

  v.emplace_back(t->data);

  auto right_v = RPQTree::inOrderNodes(t->right);
  v.insert(v.end(), right_v.begin(), right_v.end());

  return v;
}

bool RPQTree::isEquivalent(RPQTree *other) {
  auto a_clean = std::vector<std::string>();
  auto b_clean = std::vector<std::string>();
  for (auto n : inOrderNodes(this)) {
    if (n != "/") {
      a_clean.emplace_back(n);
    }
  }
  for (auto n : inOrderNodes(other)) {
    if (n != "/") {
      b_clean.emplace_back(n);
    }
  }

  if (a_clean.size() != b_clean.size()) {
    return false;
  }
  for (auto i = 0; i < a_clean.size(); i++) {
    if (a_clean[i] != b_clean[i]) {
      return false;
    }
  }

  return true;
}

