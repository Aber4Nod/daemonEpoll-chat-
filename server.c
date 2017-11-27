#include "bapi.h"

Node* clList = NULL;
int pipe_fd[2];

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

       static struct epoll_event evst, events[EPOLL_SIZE];
       static struct epoll_event evinst, eventsin[1];

       evst.events = EPOLLIN | EPOLLET;
       evinst.events = EPOLLIN | EPOLLET;

       char message[BUF_SIZE];

       int epfd, epfdin;

       int client, res, epoll_events_count;

       if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0){
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

             for(int i = 0; i < epoll_events_count ; i++){
               if(events[i].data.fd == sockfd)
               {
                   if((client = accept(sockfd, (struct sockaddr *) &useraddr, &socklen)) < 0){
                   perror("accept");
                   continue;
                   }
                   SetNB(client);
                   evst.data.fd = client;

                  if((epoll_ctl(epfd, EPOLL_CTL_ADD, client, &evst)) < 0){
                      perror("epoll_ctl_client");
                      continue;
                  }

                  if(clList == NULL){ 
                     clList = (Node*)malloc(sizeof(Node));
                     push(clList, client);
                     printf("created client (first): %d\n",clList->value);
                  } else {
                     Node* node = (Node*)malloc(sizeof(Node));
                     pushBack(clList, node, client);
                     printf("created client: %d\n",node->value);
                  }
                  memset(&message,0, BUF_SIZE);
                  res = snprintf(message, BUF_SIZE, "Welcome! Your id: %d", client);
                  while(res>0) {
                      res -= send(client, message, BUF_SIZE, 0); 
                  }
               } else {
                   HMessage(events[i].data.fd);
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

void HMessage(int client)
{
   char buf[BUF_SIZE], message[BUF_SIZE];
   memset(&buf,0,BUF_SIZE);
   memset(&message,0,BUF_SIZE);

   int len;

   len = recv(client, buf, BUF_SIZE, 0);
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
   } else {
       snprintf(message, BUF_SIZE, "User: %d|>  %s", client, buf);
       write(pipe_fd[1], message, BUF_SIZE);
       Node* elm = clList;
       while(elm != NULL){
        if (elm->value != client){
            send(elm->value, message, BUF_SIZE, 0);
         }
        elm = elm->next;
       }
   }
   return;
}

int SetNB(int fd)
{
   fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)|O_NONBLOCK);
   return 0;
}

