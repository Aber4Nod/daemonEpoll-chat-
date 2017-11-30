#include <stdlib.h>

typedef struct Message {
    char* message;
    int begin;//читать при записи отсюда
    int curlength;//вся длина текущая
    struct Message* next;
} Message;

void pushMess(Message* head, char* data){
    head->message = data;
    head->next = NULL;
    return;
}

Message* getLastMess(Message* head){
    Message* elm = head;
    if (elm == NULL){
        return NULL;
    }
    while(elm->next){
        elm = elm->next;
    }
    return elm;
}

void pushBackMess(Message* head, Message* node, char* data){
    Message* last = getLastMess(head);
    node->message = (char*)malloc(strlen(data)*sizeof(char));
    strcpy(node->message,data);
    node->begin=0;
    node->curlength=strlen(data);
    node->next = NULL;
    last->next = node;
}
void deleteMessageMess(Message* head, char* data){
    Message* elm = head;
    if (head==NULL)
        return;
    if (elm->next==NULL)
        return;
    while(elm->next && strcmp(elm->next->message,data)){
        elm = elm->next;
    }
    if (elm->next == NULL)
        return;
    Message* tmp = elm->next;
    elm->next = tmp->next;
    free(tmp);
    return;
}

