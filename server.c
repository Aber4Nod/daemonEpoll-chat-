#include "bapi.h"

Node* clList = NULL;
int pipe_fd[2];
int epfd, epfdin;
static struct epoll_event evst, events[EPOLL_SIZE];
int rdy, ttl;
char buf[BUF_SIZE];
int sender;
int main(int argc, char *argv[])
{
   if(argc<3){
       printf("invalid number of arguments\n");
       exit(-1);
   }
   char* id = argv[1];
   ushort port = atoi(argv[2]);

   int pidD = fork();
   if (!pidD) {
       setsid();
       int sockfd;

       struct sockaddr_in addr, useraddr;
       addr.sin_family = PF_INET;
       addr.sin_port = htons(port);
       addr.sin_addr.s_addr = inet_addr(id);

       socklen_t socklen;
       socklen = sizeof(struct sockaddr_in);

       static struct epoll_event evinst, eventsin[1];

       evst.events = EPOLLIN | EPOLLOUT;
       evinst.events = EPOLLIN | EPOLLET;

       char message[BUF_SIZE];


       int client, res, epoll_events_count;

       if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
           perror("socket");
           exit(-1);
       }

       SetNB(sockfd);

       if((bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))) < 0){
           perror("bind");
           exit(-1);
       }

       listen(sockfd, SOMAXCONN);

       pipe(pipe_fd);

       if((epfd = epoll_create(EPOLL_SIZE)) < 0){
           perror("epoll_create");
           exit(-1);
       }
       if((epfdin = epoll_create(1)) < 0){
           perror("epoll_createIN");
           exit(-1);
       }
      
       evst.data.fd = sockfd;
       if((epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &evst)) < 0){
           perror("epoll_ctl");
           exit(-1);
       }
       evinst.data.fd = pipe_fd[0];
       if((epoll_ctl(epfdin, EPOLL_CTL_ADD, pipe_fd[0], &evinst)) < 0){
           perror("epoll_ctl");
           exit(-1);
       }

       if (fork()) {
            close(pipe_fd[0]);
            while (1) {
             epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, EPOLL_RUN_TIMEOUT);

             for(int i = 0; i < epoll_events_count; i++){
               if(events[i].data.fd == sockfd)
               {
                   if((client = accept(sockfd, (struct sockaddr *) &useraddr, &socklen)) < 0){
                   perror("accept");
                   continue;
                   }
                   SetNB(client);
                   if (rdy>0){
                       evst.events = EPOLLOUT;
                       printf("setting cl# %d epollout\n",client);
                   } else {
                       evst.events = EPOLLIN;
                       printf("setting cl# %d epollout\n",client);
                   }
                   evst.data.fd = client;

                  if((epoll_ctl(epfd, EPOLL_CTL_ADD, client, &evst)) < 0){
                      perror("epoll_ctl_client");
                      continue;
                  }

                  if(clList == NULL){ 
                     clList = (Node*)malloc(sizeof(Node));
                     push(clList, client);
                     clList->length = 0;
                     printf("created client (first): %d\n",clList->value);
                  } else {
                     Node* node = (Node*)malloc(sizeof(Node));
                     pushBack(clList, node, client);
                     node->length = 0;
                     printf("created client: %d\n",node->value);
                  }
                  ttl++;
                 
                  memset(&message,0, BUF_SIZE);
                  res = snprintf(message, BUF_SIZE, "Welcome! Your id: %d", client);
                  send(client, message, BUF_SIZE, 0); // lets consider this without handling 
               } else {
                   HMessage(events[i].data.fd, events[i].events);
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
   } else {
       printf("daemon pid: %d\n",pidD);
       return 0;
   }
}

void HMessage(int client, uint32_t events)
{
   char buf[BUF_SIZE], message[BUF_SIZE];
   memset(&message,0,BUF_SIZE);

   if (events == EPOLLIN) {
       memset(&buf,0,BUF_SIZE);
       int len = 0;
       do {
           len = recv(client, buf, BUF_SIZE, 0);
       } while (len < 0 && (errno == EINTR || errno == EAGAIN)); // timeouts for EAGAIN probably would be more valid here
       if (len < 0) {
           perror("recv error:");
           exit(-1);
       }
       if(len == 0){
           close(client);
           if (clList->value != client) {
               deleteNode(clList,client);
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
           ttl--;
       } else {
           Node* elm = clList;
           sender = client;
           int len = snprintf(message, BUF_SIZE, "User: %d|>  %s", sender, buf);
           write(pipe_fd[1], message, len);
           if (ttl>1){
               evst.events = EPOLLIN;
               evst.data.fd = client;
               epoll_ctl(epfd, EPOLL_CTL_DEL, client, &evst);
               while(elm != NULL){
                   evst.events = EPOLLOUT;
                   evst.data.fd = elm->value;
                   epoll_ctl(epfd, EPOLL_CTL_MOD, elm->value, &evst);
                   elm = elm->next;
               }
           }
        }
       } else if (events == EPOLLOUT) {
           int len = snprintf(message, BUF_SIZE, "User: %d|>  %s", sender, buf);
           Node* elm = clList;
           while(elm != NULL){
            if (elm->value == client){
               int ret = send(client, message+elm->length, len-elm->length, 0);
               if (ret == -1 && errno == EINTR || ret < len) {
                   if (ret != -1){
                      elm->length += len - ret;
                      rdy++;
                   }
               } else {
                   evst.events = EPOLLOUT;
                   evst.data.fd = client;
                   epoll_ctl(epfd, EPOLL_CTL_DEL, client, &evst);
                   elm->length = 0;
                   rdy--;
               }
             }
            elm = elm->next;
           }
           if (rdy<=0){
               elm = clList;
               while(elm != NULL){
                   evst.events = EPOLLIN;
                   evst.data.fd = elm->value;
                   epoll_ctl(epfd, EPOLL_CTL_ADD, elm->value, &evst);
                   elm = elm->next;
               }
               rdy = 0;
           }
       }
   return;
}

int SetNB(int fd)
{
   fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)|O_NONBLOCK);
   return 0;
}

