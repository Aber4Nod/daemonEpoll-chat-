#include <stdlib.h>
#include "custMess.h"
typedef struct Node {
    /* char* message; */
    int writable;
    Message* message;
    /* int begin;//читать при записи отсюда */
    int fd;
    /* int rest;//сколько осталось до конца массива от последней записи */
    /* int end;//читать при записи до сюда (конец смысловых данных) */
    /* int curlength;//вся длина текущая */
    struct Node* next;
} Node;

void push(Node* head, int data){
    head->fd = data;
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
    node->fd = value;
    node->next = NULL;
    last->next = node;
}
void deleteNode(Node* head, int value){
    Node* elm = head;
    if (head==NULL)
        return;
    if (elm->next==NULL)
        return;
    while(elm->next && elm->next->fd!=value){
        elm = elm->next;
    }
    if (elm->next == NULL)
        return;
    Node* tmp = elm->next;
    elm->next = tmp->next;
    free(tmp);
    return;
}
