#include "bapi.h"

char message[BUF_SIZE];
int restLength;
int main(int argc, char *argv[])
{
   if(argc<3){
       printf("invalid number of arguments\n");
       exit(-1);
   }
   char* id = argv[1];
   ushort port = atoi(argv[2]);


   int sockfd, pipefd[2], evfd;

   struct sockaddr_in addr;
   addr.sin_family = AF_INET;
   addr.sin_port = htons(port);
   addr.sin_addr.s_addr = inet_addr(id);

   static struct epoll_event ev, events[2]; 
   ev.events = EPOLLIN | EPOLLET;

   int continue_to_work = 1;

   if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
       perror("socket");
       exit(-1);
   }
   if((connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))) < 0){
       perror("connect");
       exit(-1);
   }
   pipe(pipefd);

   if((evfd = epoll_create(EPOLL_SIZE)) < 0){
       perror("epoll_create");
       exit(-1);
   }
   ev.data.fd = sockfd;
   if((epoll_ctl(evfd, EPOLL_CTL_ADD, sockfd, &ev)) < 0){
       perror("epoll_ctl");
       exit(-1);
   }

   ev.data.fd = pipefd[0];
   if((epoll_ctl(evfd, EPOLL_CTL_ADD, pipefd[0], &ev)) < 0){
       perror("epoll_ctl_IN");
       exit(-1);
   }

   if(fork()){
       close(pipefd[1]); 
       int epoll_events_count, res;

       while(continue_to_work) {
           epoll_events_count = epoll_wait(evfd, events, 2, EPOLL_RUN_TIMEOUT);

           for(int i = 0; i < epoll_events_count ; i++){
               bzero(&message, BUF_SIZE);

               if(events[i].data.fd == sockfd){
                       do {
                           res = recv(sockfd, message, BUF_SIZE, 0);
                       } while (res < 0 && (errno == EINTR || errno == EAGAIN));

                       if (res < 0) {
                           perror("recv error:");
                           exit(-1);
                       }
                       if(res == 0){
                           close(sockfd);
                           continue_to_work = 0;
                       }
                       else
                           printf("%s\n", message);
               } else {
                       res = read(events[i].data.fd, message, BUF_SIZE);
                       if (res == 0) 
                           continue_to_work = 0;
                       int ret;
                       do {
                           ret = send(sockfd, message + restLength, res-restLength, 0);
                           if (ret>0)
                               restLength += res - ret;
                       } while (ret == -1 && errno == EINTR || ret < res);
                   if (ret < 0) {
                           perror("send:");
                           exit(-1);
                           restLength = 0;
                       }
               }
           }
       }
       close(pipefd[0]);
       close(sockfd);
   } else {
       close(pipefd[0]); 
       printf("Input 'exit' to exit\n");
       while(continue_to_work){
          bzero(&message, BUF_SIZE);
          fgets(message, BUF_SIZE, stdin);

          if(strncasecmp(message, "exit", 4) == 0){
              continue_to_work = 0;
          } else {
              write(pipefd[1], message, strlen(message) - 1);
          }
       }
       close(pipefd[1]);
   }
   return 0;
}
