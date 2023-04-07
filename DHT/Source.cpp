#include <iostream>
#include <math.h>
#include "Header.h"

#define m 8

void Node::betterprint() {
    Node* succ = this->fingerTable_.get(1);
    std::cout << "----------Node id:" << +this->fingerTable_.getnodeId_() << "-------------\n";
    std::cout << "Successor:   " << +succ->getId() << " Predecessor:   " << +this->predecessor->id_ << "\n";
    std::cout << "FingerTables:" << "\n";
    int startinginterval = 1 + this->id_;
    for (int i = 1; i <= m; i++) {
        Node* n = this->fingerTable_.get(i);
        int finishinginterval = this->startprint(this->id_, i);
        std::cout << "| k = " << i << " [ " << startinginterval << " , " << finishinginterval << " )      succ. = " << +n->getId() << " |\n";
        startinginterval = finishinginterval;
    }
    std::cout << "--------------------------------" << "\n";
    std::cout << "********************************" << "\n";

}
int Node::startprint(uint8_t id, int i) {
    int val = id + pow(2, i); 
    int val2 = pow(2, m);
    int val3 = val % val2;
    return val3;
}

void Node::join(Node* node) {
    this->fingerTable_ = FingerTable(this->id_);
    if (node == NULL) {
        for (int i = 1; i <= m;i++) {
            this->fingerTable_.set(i, this);
        }
        this->predecessor = this;
        this->prettyPrint();
    }
    else {

        this->initfingertable(node);
        this->update_others();
        this->prettyPrint();
        Node* succ = this->fingerTable_.get(1);
        for (auto i = succ->localKeys_.begin(); i != succ->localKeys_.end();) {
            if (i->first <= this->id_ && this->id_ < succ->id_) {
                this->localKeys_.insert({ i->first, i->second });
                std::cout << "migrate key " << +i->first << " from node " << +succ->id_ << " to node " << +this->id_ << "\n";
                succ->localKeys_.erase(i++);
            }
            else {
                ++i;
            }
        }
    }
}
uint8_t Node::find(uint8_t key) 
{
    Node* N = this->find_successor(key);
    auto i = N->localKeys_.find(key);
    if (i != N->localKeys_.end()) {
        if (this->id_ == N->id_) {
            if (i->second == NULL) {
                std::cout << "Look-up result of key " << +key << " from node " << +this->id_ << " with path [" << +N->id_ << "] value is None \n";
            }
            else {
                std::cout << "Look-up result of key " << +key << " from node " << +this->id_ << " with path [" << +this->id_ << "] value is " << +i->second << "\n";
            }
        }
        else if (i->second == NULL) {
            std::cout << "Look-up result of key " << +key << " from node " << +this->id_ << " with path [" << +this->id_ << "," << +N->id_ << "] value is None \n";
        }
        else {
            std::cout << "Look-up result of key " << +key << " from node " << +this->id_ << " with path [" << +this->id_ << "," << +N->id_ << "] value is " << +i->second << "\n";
        }
        return i->second;
    }
    else {
        return 0;
    }
}
void Node::insert(uint8_t key, uint8_t val)
{
    Node* succ = this->find_successor(key);
    succ->localKeys_.insert({ key, val });
    this->printKey();
}
void Node::remove(uint8_t key)
{
    Node* N = this->find_successor(key);
    N->localKeys_.erase(key);
    std::cout << "removed key " << key << "from node " << N->id_;
}
int Node::start(uint8_t id, int i) {
    int val = id + pow(2, i - 1); // copying other finger table
    int val2 = pow(2, m);
    int val3 = val % val2;
    return val3;
}
int inRangefing(uint8_t succid, uint8_t currid, uint8_t id) {
    if (currid >= id) {
        return succid > currid || succid < id;
    }
    else {
        return succid > currid && succid < id;
    }
}
void Node::initfingertable(Node* node)
{
    uint8_t val3 = this->start(this->id_, 1);
    this->fingerTable_.set(1, node->find_successor(val3)); //finger[1].node = successor
    Node* successor = this->fingerTable_.get(1); //finger1node = successor
    this->predecessor = successor->predecessor;
    successor->predecessor = this;
    for (int i = 1; i < m; i++) {
        successor = this->fingerTable_.get(i); //finger[i].node
        uint8_t val3 = this->start(this->id_, i + 1);
        if (inRangefing(this->id_, successor->id_, val3)) { //if finger[i+1].start is in range (n,finger[i].node)
            this->fingerTable_.set(i + 1, successor);
        }
        else {
            Node* ind = node->find_successor(val3);
            this->fingerTable_.set(i + 1, ind);
        }
    }
}

Node* Node::closest_preceding_finger(uint8_t id)
{
    for (int i = m; i > 0; i--) {
        Node* N = this->fingerTable_.get(i);
        int succid = N->getId();
        if (inRangefing(succid, this->id_, id)) { // finger[i].node E (N, id) 
            return N;
        }
    }
    return this;
}
bool inRange(uint8_t id, uint8_t currid, uint8_t succid) {
    if (currid >= succid) {
        return id <= currid && id > succid;
    }
    else {
        return id <= currid || id > succid;
    }
}
Node* Node::find_predecessor(uint8_t id)
{
    Node* N = this;
    Node* successor = this->fingerTable_.get(1);
    uint8_t currid = N->id_;
    uint8_t succid = successor->id_;
    if (this->id_ == successor->id_) { //only one node in the network
        return this;
    }
    while (inRange(id, currid, succid)) { 
        N = N->closest_preceding_finger(id);
        currid = N->id_;
        successor = N->fingerTable_.get(1);
        succid = successor->id_;
    }
    return N;
}
Node* Node::find_successor(uint8_t id)
{
    Node* N = this->find_predecessor(id);
    return N->fingerTable_.get(1);
}

void Node::update_others() {
    for (int i = 1; i <= m; i++) {
        Node* p = this->find_predecessor(this->id_ - pow(2, i - 1));
        p->update_finger_table(this, i);
    }
}

void Node::prettyPrint() {
    Node* x = this->find_successor(pow(2, m) - 1);
    Node* n = x;
    n->betterprint();
    n = n->fingerTable_.get(1);
    while (x->id_ != n->id_) {
        n->betterprint();
        n = n->fingerTable_.get(1);
    }
}

void Node::printKey()
{
    Node* start = this->find_successor(pow(2, m) - 1);
    std::cout << "------------ Node id:" << +start->id_ << "-----------" << "\n";
    std::cout << "{";
    int comma = 0;
    for (auto i = start->localKeys_.begin(); i != start->localKeys_.end(); ++i) {
        if (comma != 0) {
            std::cout << ", ";
        }
        if (i->second == NULL) {
            std::cout << +i->first << ": None";
        }
        else {
            std::cout << +i->first << ": " << +i->second;
        }
        comma++;
    }
    std::cout << "}" << "\n";
    Node* n = start->fingerTable_.get(1);
    while (start->id_ != n->id_) {
        std::cout << "------------ Node id:" << +n->id_ << "-----------" << "\n";
        std::cout << "{";
        comma = 0;
        for (auto i = n->localKeys_.begin(); i != n->localKeys_.end(); ++i) {
            if (comma != 0) {
                std::cout << ", ";
            }
            if (i->second == NULL) {
                std::cout << +i->first << ": None";
            }
            else {
                std::cout << +i->first << ": " << +i->second;
            }
            comma++;
        }
        std::cout << "}" << "\n";
        n = n->fingerTable_.get(1);
    }
}

int updateInRange(uint8_t currid, uint8_t succid, uint8_t id) {
    if (currid >= succid) {
        return id > currid || id < succid;
    }
    else {
        return id > currid && id < succid;
    }
}
void Node::update_finger_table(Node* s, int i) {
    Node* fing = this->fingerTable_.get(i);
    int ind = fing->id_;
    if (updateInRange(this->id_, fing->id_, s->id_)) {
        this->fingerTable_.set(i, s);
        Node* p = this->predecessor;
        p->update_finger_table(s, i);
    }
}

int main() {
    Node n0(0);
    Node n1(30);
    Node n2(65);
    Node n3(110);
    Node n4(160);
    Node n5(230);
    Node n6(100);
    n0.join(NULL);
    n1.join(&n0);
    n2.join(&n1);
    n3.join(&n2);
    n4.join(&n3);
    n5.join(&n4);
    n0.insert(3, 3);
    n1.insert(200);
    n2.insert(123);
    n3.insert(45, 3);
    n4.insert(99);
    n2.insert(60, 10);
    n0.insert(50, 8);
    n3.insert(100, 5);
    n3.insert(101, 4);
    n3.insert(102, 6);
    n5.insert(240, 8);
    n5.insert(250, 10);
    n6.join(&n5);
    n6.find(3);
    n6.find(200);
    n6.find(123);
    n6.find(45);
    n6.find(99);
    n6.find(60);
    n6.find(50);
    n6.find(100);
    n6.find(101);
    n6.find(102);
    n6.find(240);
    n6.find(250);
    return 0;
}