/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

/**
 Linux (POSIX) implementation of _kbhit().
 Morgan McGuire, morgan@cs.brown.edu
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
//#include <curses.h>
#include <arpa/inet.h>



/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char buf[10];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }
  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

  /* citirea mesajului */
    int login=0, nrlogs=0;
    char log_ans[200];
    while(login==0){

      nrlogs++;
      if (write (sd,&nrlogs,sizeof(int)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
       
    printf("NRLOGS: %d \n", nrlogs);

      /* citirea raspunsului dat de server 
         (apel blocant pana cind serverul raspunde) */
      if (read (sd, log_ans,sizeof(log_ans)) < 0)
        {
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
        }
        printf("%s\n", log_ans);
      fflush (stdout);
      printf("User: ");
      scanf("%s",buf);
      //buf[strlen(buf)-1]='\0';
      int len=strlen(buf);
      if (write (sd,&len,sizeof(int)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
            }      
      /* trimiterea mesajului la server */
      if (write (sd,buf,len) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
            }

      printf ("Password: ");
      fflush (stdout);
      int ok=1;
      char passw[255]={0};
        
        scanf("%s", passw);
        len=strlen(passw);
      /* trimiterea mesajului la server */
      if (write (sd,&len,sizeof(int)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
            }  

      if (write (sd,passw,len) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        
    

  /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
  if (read (sd, &nr,sizeof(int)) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }
  login=nr;
}
        
    if(login==1){

        while(1){

      printf (" You can:\n 1) Interrogate other PCs state \n 2) Shutdown a computer \n 3) Execute bash commands \n 4) Quit \n Answer: ");
      fflush (stdout);
      
      scanf("%s",buf);
            
      /* trimiterea mesajului la server */
      if (write (sd,buf,sizeof(buf)) <= 0)
        {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
            }

        

        if(strcmp(buf,"1")==0){

            int Len;
            char message[10000]="";
            if (read (sd, &Len,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if (read (sd, message, Len) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }

            printf("%s", message);
        }

        if(strcmp(buf, "2")==0){

            int Len;
            char message[10000]="";
            printf("Choose which PC you want to close: \n");
            if (read (sd, &Len,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if (read (sd, message, Len) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }

            printf("%s", message);
            int nr_ip;
            scanf("%d", &nr_ip);

            if (write (sd,&nr_ip,sizeof(int)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }
            int L1;
            char msg[100]="";
            if (read (sd, &L1,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if (read (sd, msg, L1) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }

            printf("%s", msg);
            
        }

        if(strcmp(buf, "3")==0){

            int Len;
            char message[10000]="";
            printf("Choose PC: \n");
            if (read (sd, &Len,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if (read (sd, message, Len) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }

            printf("%s", message);
            int nr_ip;
            scanf("%d", &nr_ip);
            

            if (write (sd,&nr_ip,sizeof(int)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }
                  
            char input[1000];
            printf("INPUT: ");
            scanf(" %[^\t\n]s",input);
            int length=strlen(input);
            if (write (sd,&length,sizeof(int)) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }  

            if (write (sd,input,length) <= 0)
            {
                perror ("[client]Eroare la write() spre server.\n");
                return errno;
            }

            char output[100000]="";

            if (read (sd, &length,sizeof(int)) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }
            if (read (sd, output, length) < 0)
            {
                perror ("[client]Eroare la read() de la server.\n");
                return errno;
            }

            printf("\nOUTPUT: %s\n", output);

        }

        if(strcmp(buf, "4")==0){

            printf("Closing client-server connection\n");
            close (sd);
            return 0;
        }

    }
    }
}

