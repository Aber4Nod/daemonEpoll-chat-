#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node* next;
} Node;

void push(Node* head, int data){
    head->value = data;
    head->next = NULL;
    return;
}

Node* getLast(Node* head){
    Node* elm = head;
    if (elm == NULL){
        return NULL;
    }
    while(elm->next){
        elm = elm->next;
    }
    return elm;
}

void pushBack(Node* head, Node* node, int value){
    Node* last = getLast(head);
    node->value = value;
    node->next = NULL;
    last->next = node;
}
void deleteNode(Node* head, int value){
    Node* elm = head;
    if (head==NULL)
        return;
    if (elm->next==NULL)
        return;
    while(elm->next && elm->next->value!=value){
        elm = elm->next;
    }
    if (elm->next == NULL)
        return;
    Node* tmp = elm->next;
    elm->next = tmp->next;
    free(tmp);
    return;
}
