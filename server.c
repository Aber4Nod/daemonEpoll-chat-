#include "bapi.h"

Node* clList = NULL;
int pipe_fd[2];
int epfd, epfdin;
static struct epoll_event events[EPOLL_SIZE];
int main(int argc, char *argv[])
{
   if(argc<3){
       printf("invalid number of arguments\n");
       exit(-1);
   }
   char* id = argv[1];
   ushort port = atoi(argv[2]);

   if(fork()) exit(0);
   setsid();
   if(fork()) exit(0);
   close(0);
   close(1);
   close(2);
   int sockfd;

   struct sockaddr_in addr, useraddr;
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   addr.sin_addr.s_addr = inet_addr(id);

   socklen_t socklen;
   socklen = sizeof(struct sockaddr_in);

   static struct epoll_event evinst, eventsin[1];

   evinst.events = EPOLLIN | EPOLLET;

   char message[BUF_SIZE];


   int client, res, epoll_events_count;

   if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
       perror("socket");
       exit(-1);
   }
   int enable;
   if(setsockopt(sockfd,SOL_SOCKET, SO_REUSEADDR,&enable,sizeof(int)) < 0)
       perror("setsockopt");

   SetNB(sockfd);

   if((bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))) < 0){
       perror("bind");
       exit(-1);
   }

   if(listen(sockfd, SOMAXCONN) < 0){
       perror("listen:");
       exit(-1);
   }

   pipe(pipe_fd);

   if((epfd = epoll_create(EPOLL_SIZE)) < 0){
       perror("epoll_create");
       exit(-1);
   }
   if((epfdin = epoll_create(1)) < 0){
       perror("epoll_createIN");
       exit(-1);
   }
   
   modifyEpollContext(epfd,EPOLL_CTL_ADD,sockfd,EPOLLIN|EPOLLET,&sockfd);
   evinst.data.fd = pipe_fd[0];
   if((epoll_ctl(epfdin, EPOLL_CTL_ADD, pipe_fd[0], &evinst)) < 0){
       perror("epoll_ctl");
       exit(-1);
   }

<<<<<<< HEAD
   if (fork()) {
        close(pipe_fd[0]);
        while (1) {
         epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_RUN_TIMEOUT);
=======
             for(int i = 0; i < epoll_events_count; i++){
               if(events[i].data.ptr == &sockfd)
               {
                  do {
                     if ((client = accept(sockfd, (struct sockaddr *) &useraddr, &socklen)) < 0) {
                     break;
                     }
                     SetNB(client);
                     Node* node;
                     if(clList == NULL){ 
                        clList = (Node*)malloc(sizeof(Node));
                        node = clList;
                        push(clList, client);
                        printf("created client (first): %d\n",clList->fd);
                     } else {
                        node = (Node*)malloc(sizeof(Node));
                        pushBack(clList, node, client);
                        printf("created client: %d\n",node->fd);
                     }
                     node->fd = client;
                     node->writable=0;
                     modifyEpollContext(epfd,EPOLL_CTL_ADD,node->fd,EPOLLIN|EPOLLET,node);
>>>>>>> b5dea848a959018281f8350fcc31925ed9679e4d

         for(int i = 0; i < epoll_events_count; i++){
           if(events[i].data.ptr == &sockfd)
           {
              do {
                 if ((client = accept(sockfd, (struct sockaddr *) &useraddr, &socklen)) < 0) {
                 /* perror("accept"); */
                 break;
                 }
                 SetNB(client);
                 Node* node;
                 if(clList == NULL){ 
                    clList = (Node*)malloc(sizeof(Node));
                    node = clList;
                    push(clList, client);
                    /* printf("created client (first): %d\n",clList->fd); */
                 } else {
                    node = (Node*)malloc(sizeof(Node));
                    pushBack(clList, node, client);
                    /* printf("created client: %d\n",node->fd); */
                 }
                 node->fd = client;
                 node->writable=0;
                 modifyEpollContext(epfd,EPOLL_CTL_ADD,node->fd,EPOLLIN,node);

                 memset(&message,0, BUF_SIZE);
                 res = snprintf(message, BUF_SIZE, "Welcome! Your id: %d\n", node->fd);
                 send(node->fd, message, res, 0); // lets consider this without handling 
              } while(1);
           } else {
               HMessage(events[i].data.ptr, events[i].events);
           }
         }
        }
        waitpid(-1,NULL,0);
        close(pipe_fd[1]);
   } else {
      close(pipe_fd[1]);
      int continue_to_work = 1;
      int fd = open("log.txt",O_WRONLY|O_CREAT|O_APPEND,0644);
      while (continue_to_work) {
         epoll_events_count = epoll_wait(epfdin, eventsin, 1, EPOLL_RUN_TIMEOUT);
         for (int i=0;i<epoll_events_count;i++) {
             if (eventsin[i].data.fd == pipe_fd[0]) {
                  memset(&message,0,BUF_SIZE);
                  res = read(eventsin[i].data.fd, message, BUF_SIZE);
                  if (res == 0) continue_to_work = 0;
                  else {
                      write(fd,message,BUF_SIZE);
                      write(fd,"\n",1);
                  }
             }
         }
      }
      close(pipe_fd[0]);
      close(fd);
   }
   close(sockfd);
   close(epfd);
   close(epfdin);
   return 0;
}

void HMessage(void* ptr, uint32_t events)
{
   Node* clData = (Node*)ptr;
   char buf[BUF_SIZE];
   char* message = (char*)malloc(BUF_SIZE*sizeof(char));
   memset(message,0,BUF_SIZE);

   if (events & EPOLLIN) {
       memset(&buf,0,BUF_SIZE);
       int _lenO = 0;
       int len = snprintf(message,BUF_SIZE,"User: %d|>  ", clData->fd);
       do {
           if ((_lenO = recv(clData->fd, buf, BUF_SIZE, 0)) > 0) {
               message = (char*)realloc(message,(_lenO+len)*sizeof(char*));
               strncat(message,buf,_lenO);
               len = strlen(message);
           }
       } while (_lenO < 0 && (errno == EINTR || errno == EAGAIN) || _lenO == BUF_SIZE); // timeouts for EAGAIN probably would be more valid here
       if (_lenO < 0) {
           perror("recv error:");
           exit(-1);
       }
       if(_lenO == 0){
           close(clData->fd);
           if (clList->fd != clData->fd) {
               deleteNode(clList,clData->fd);
           } else {
               Node* elm = clList;
               if (elm->next == NULL) {
                   free(elm);
                   clList = NULL;
               } else {
                   clList = clList->next;
                   free(elm);
               }
           }
       } else {
           Node* elm = clList;
           write(pipe_fd[1], message, len);
           while(elm != NULL){
               if (elm->fd != clData->fd){
                   if (elm->message == NULL)
                   {
                     elm->message = (Message*)malloc(sizeof(Message));
                     pushMess(elm->message,(char*)malloc(len*sizeof(char)));
                     elm->message->begin=0;
                     elm->message->curlength=len;
                     strcpy(elm->message->message,message);
                   } else {
                       Message* newmsg = (Message*)malloc(sizeof(Message));
                       pushBackMess(elm->message,newmsg,message);
                   }
                   if (elm->writable){
                       do {
                        int ret = send(elm->fd, elm->message->message+elm->message->begin, elm->message->curlength-elm->message->begin, 0);
                        if (ret == -1 && errno == EINTR || ret < elm->message->curlength-elm->message->begin) {
                           if (ret != -1){
                              elm->message->begin += elm->message->curlength - ret;
                              elm->writable = 0;
                              modifyEpollContext(epfd, EPOLL_CTL_MOD, elm->fd, EPOLLOUT|EPOLLET, elm);
                              break;
                           }
                        } else {
       /* printf("client #%d in 2\n",clData->fd); */
                           Message* _bufmes = elm->message;
                           if (_bufmes->next != NULL){
                               _bufmes = _bufmes->next;
                               free(elm->message);
                           } else {
                               free(elm->message);
                               elm->message=NULL;
                           }
                        }
                       } while (elm->message != NULL);
                   } else {
                       modifyEpollContext(epfd, EPOLL_CTL_MOD, elm->fd, EPOLLOUT|EPOLLET, elm);
                   }
               }
               elm=elm->next;
           }
        }
       } else if (events & EPOLLOUT) {
           modifyEpollContext(epfd, EPOLL_CTL_MOD, clData->fd, EPOLLIN|EPOLLET, clData);
       /* printf("client #%d in OUT\n",clData->fd); */
          if (clData->message != NULL) 
           do {
            int ret = send(clData->fd, clData->message->message+clData->message->begin, clData->message->curlength-clData->message->begin, 0);
       /* printf("client #%d in 1\n",clData->fd); */
            if (ret == -1 && errno == EINTR || ret < clData->message->curlength-clData->message->begin) {
               if (ret != -1){
                  clData->message->begin += clData->message->curlength - ret;
                  break;
               }
            } else {
               Message* _bufmes = clData->message;
               if (_bufmes->next != NULL){
                   _bufmes = _bufmes->next;
                   free(clData->message);
               } else {
                   free(clData->message);
                   clData->message=NULL;
               }
            }
           } while (clData->message != NULL);
           if (clData->message == NULL)
               clData->writable = 1;
       }
   return;
}

int SetNB(int fd)
{
   fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)|O_NONBLOCK);
   return 0;
}

void modifyEpollContext(int epollfd, int operation, int fd, uint32_t events, void* data)
{
    struct epoll_event server_listen_event;
    server_listen_event.events = events;
    server_listen_event.data.ptr = data;
    if (epoll_ctl(epollfd,operation,fd,&server_listen_event))
    {
        perror("ctl:");
        exit(-1);
    }
}
