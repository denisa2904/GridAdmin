/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sqlite3.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
int login(void*);
void Create_table();
void Insert_filects();
void Update_cts();
int callback(void *, int, char **, char **);

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  
    Create_table();
    Insert_filects();
  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    

  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}

    char *IP = inet_ntoa(from.sin_addr);
    printf("IP: %s\n",IP);

	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;    
	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};			

	
void Create_table(){

    sqlite3 *db;
    char *err_msg = 0;
    
    int rc = sqlite3_open("connection.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return ;
    }
    
    char *sql = "DROP TABLE IF EXISTS Conn;" 
                "CREATE TABLE Conn(IP TEXT, State TEXT, User TEXT);";
  
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        
        return ;
    } 
    sqlite3_close(db);
}

void Insert_filects()
{
    sqlite3 *db;
    char *err_msg = 0;
    
    int rc = sqlite3_open("connection.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return ;
    }

    FILE *fp=fopen("ip_addrs.txt","r");
    if(fp==NULL){
    perror("Unable to open file!");
    exit(1);
    }

    char ip[128];
    char user[10];
    while(fgets(ip, sizeof(ip),fp)!=NULL){
        
        ip[strlen(ip)-1]='\0';
        if(strcmp(ip, "192.168.253.5")==0) strcpy(user,"denisa");
        if(strcmp(ip, "192.168.253.6")==0) strcpy(user,"denisa1");        
        char insert_ip[1000];
        sprintf(insert_ip, "INSERT INTO Conn VALUES('%s','off','%s');", ip, user);
        rc=sqlite3_exec(db, insert_ip,0,0,&err_msg);
            
    }  
    fclose(fp);
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        
        return ;
    } 
    sqlite3_close(db);
}

void Update_cts(){

    sqlite3 *db;
    char *err_msg = 0;
    
    int rc = sqlite3_open("connection.db", &db);
    
    if (rc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return ;
    }

    FILE *fp=fopen("ip_addrs.txt","r");
    if(fp==NULL){
    perror("Unable to open file!");
    exit(1);
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    char ip[128];
    while(fgets(ip, sizeof(ip),fp)!=NULL){

        ip[strlen(ip)-1]='\0';
        char insert_ip[1000];
        char sir[1000];
        char state[5];
        struct sockaddr_in fileip;
        fileip.sin_family=AF_INET;
        fileip.sin_port=htons(4496);
        inet_pton(AF_INET, ip, &fileip.sin_addr);
        sprintf(insert_ip, "UPDATE Conn SET State = 'off' WHERE IP = '%s';", ip);
        rc=sqlite3_exec(db, insert_ip,0,0,&err_msg);
        strcpy(state,"on");
        if (connect(sockfd, (struct sockaddr *) &fileip, sizeof(fileip)) == -1)
        {
            //sprintf(sir,"connecting %s: %d (%s) ", ip, errno, strerror(errno));
            //printf("\n connecting %s: %d (%s) \n", ip, errno, strerror(errno));
            //if(errno==111||errno==106) strcpy(state, "on");
            if(errno==113)strcpy(state,"off");
            //printf(" %d %s %s\n",errno, state, strerror(errno));
        }
        //else printf("%s %s\n", ip, state);
        sprintf(insert_ip, "UPDATE Conn SET State = '%s' WHERE IP = '%s';", state, ip);
        rc=sqlite3_exec(db, insert_ip,0,0,&err_msg);
            
    }    
    fclose(fp);
    if (rc != SQLITE_OK ) {
        
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        
        return ;
    } 
    sqlite3_close(db);

}

static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Connecting...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());
        if(login((struct thData*)arg)==1){	
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);}
		return(NULL);	
  		
};



int login(void *arg)
{

        int i=0, ans=0, nrlogs=0;
        struct thData tdL; 
	    tdL= *((struct thData*)arg);
        while(ans==0){

        if (read (tdL.cl, &nrlogs, sizeof(int)) <= 0)
			    {
			      printf("[Thread %d]\n",tdL.idThread);
			      perror ("Eroare la read() de la client.\n");
			    
			    }
        printf("NRLOGS:%d\n",nrlogs);
        if(nrlogs==4) {
            char log[200]="Client-server connection is closing! You have more than 3 wrong login attempts";
            if (write (tdL.cl, log, sizeof(log)) <= 0)
		    {
		        printf("[Thread %d] ",tdL.idThread);
		        perror ("[Thread]Eroare la write() catre client.\n");
		    }           
            /* am terminat cu acest client, inchidem conexiunea */
		    close ((intptr_t)arg);
        }
        else{
            char log[100];
            int logatt=4-nrlogs;
            sprintf(log, "you have %d chances to login", logatt);
            if (write (tdL.cl, log, sizeof(log)) <= 0)
		    {
		        printf("[Thread %d] ",tdL.idThread);
		        perror ("[Thread]Eroare la write() catre client.\n");
		    }           
        }
        int len;
        if (read (tdL.cl, &len , sizeof(int)) <= 0)
			    {
			      printf("[Thread %d]\n",tdL.idThread);
			      perror ("Eroare la read() de la client.\n");
			    
			    }
        char user[len];
	    if (read (tdL.cl, user, len) <= 0)
			    {
			      printf("[Thread %d]\n",tdL.idThread);
			      perror ("Eroare la read() de la client.\n");
			    
			    }
        user[len]='\0';
        if (read (tdL.cl, &len , sizeof(int)) <= 0)
			    {
			      printf("[Thread %d]\n",tdL.idThread);
			      perror ("Eroare la read() de la client.\n");
			    
			    }
        char passw[len];
        if (read (tdL.cl, passw, len) <= 0)
			    {
			      printf("[Thread %d]\n",tdL.idThread);
			      perror ("Eroare la read() de la client.\n");
			    
			    }
        passw[len]='\0';
        printf("| %s | %s |\n", user, passw);
        
        if(strcmp(user, "admin")==0 && strcmp(passw, "adminpass")==0) ans=1;
	    
		          
		          /*pregatim mesajul de raspuns */
		                
	    printf("[Thread %d]Authentification code %d\n",tdL.idThread, ans);
		          
		      
		      /* returnam mesajul clientului */
	 if (write (tdL.cl, &ans, sizeof(int)) <= 0)
		{
		 printf("[Thread %d] ",tdL.idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
	else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);	
    }
}

void raspunde(void *arg)
{
        while(1){

        int i=0;
        char comanda[100];
	    struct thData tdL; 
	    tdL= *((struct thData*)arg);
	    if (read (tdL.cl, comanda, 100) <= 0)
			    {
			      printf("[Thread %d]\n",tdL.idThread);
			      perror ("Eroare la read() de la client.\n");
			    
			    }
        
	    
	    printf ("[Thread %d]Mesajul a fost receptionat...\n",tdL.idThread);
        //printf("|%s|\n",comanda);
		          
		          /*pregatim mesajul de raspuns */
		                
	    printf("[Thread %d]Trimitem mesajul inapoi...\n",tdL.idThread);
		          
        printf("\n\n");

        char message[10000]="";
        int mlen;
        if(strcmp(comanda,"1")==0)
        {
            //PCs state
            Update_cts();
            sqlite3 *db;
            char *err_msg = 0;
            sqlite3_stmt* stmt;
            
            if (sqlite3_open ("connection.db", &db) != SQLITE_OK) {
                printf("Error opening database.\n");
                return ;
            }
            
            char *sql = "SELECT * FROM Conn;";
                
            int rc= sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            if(rc!=SQLITE_OK){
                printf("Error fetching data");
                sqlite3_close(db);
            }
            char row[1000];
            while(sqlite3_step(stmt)==SQLITE_ROW){

                sprintf(row,"%s %s\n", sqlite3_column_text(stmt,0), sqlite3_column_text(stmt,1));
                strcat(message, row);
            }
            mlen=strlen(message);
            sqlite3_finalize(stmt);
            sqlite3_close(db);

            printf("%s\n", message);

            if (write (tdL.cl, &mlen, sizeof(int)) <= 0)
		    {
		     printf("[Thread %d] ",tdL.idThread);
		     perror ("[Thread]Eroare la write() catre client.\n");
		    }
	        else
		        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
    
            if (write (tdL.cl, message, mlen) <= 0)
		    {
		     printf("[Thread %d] ",tdL.idThread);
		     perror ("[Thread]Eroare la write() catre client.\n");
		    }
	        else
		        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);


        }
        strcpy(message,"");

        if(strcmp(comanda, "2")==0){
            //close PC
            //Update_cts();
            sqlite3 *db;
            char *err_msg = 0;
            sqlite3_stmt* stmt;
            
            if (sqlite3_open ("connection.db", &db) != SQLITE_OK) {
                printf("Error opening database.\n");
                return ;
            }
            
            char *sql = "SELECT * FROM Conn;";
                
            int rc= sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            if(rc!=SQLITE_OK){
                printf("Error fetching data");
                sqlite3_close(db);
            }
            char row[1000];
            while(sqlite3_step(stmt)==SQLITE_ROW){

                sprintf(row,"%s\n", sqlite3_column_text(stmt,0));
                strcat(message, row);
            }
            mlen=strlen(message);
            sqlite3_finalize(stmt);
            sqlite3_close(db);

            if (write (tdL.cl, &mlen, sizeof(int)) <= 0)
		    {
		     printf("[Thread %d] ",tdL.idThread);
		     perror ("[Thread]Eroare la write() catre client.\n");
		    }
	        else
		        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
    
            if (write (tdL.cl, message, mlen) <= 0)
		    {
		     printf("[Thread %d] ",tdL.idThread);
		     perror ("[Thread]Eroare la write() catre client.\n");
		    }
	        else
		        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);

            int nr_ip;
            if (read (tdL.cl, &nr_ip, sizeof(int)) <= 0)
			    {
			      printf("[Thread %d]\n",tdL.idThread);
			      perror ("Eroare la READ() de la client.\n");
			    
			    }
            printf("%d\n", nr_ip);

            //Update_cts();
            
            if (sqlite3_open ("connection.db", &db) != SQLITE_OK) {
                printf("Error opening database.\n");
                return ;
            }
            
            sql = "SELECT * FROM Conn;";
                
            rc= sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            if(rc!=SQLITE_OK){
                printf("Error fetching data");
                sqlite3_close(db);
            }
            int i=0, Close=0;
            char chosenIP[]="Couldn't close this PC";
            char IP2[100], chosen_user[10];
            while(sqlite3_step(stmt)==SQLITE_ROW){
                i++;
                    sprintf(chosenIP,"Closing %s \n", sqlite3_column_text(stmt,0)); 
                    sprintf(IP2,"%s", sqlite3_column_text(stmt,0));
                    sprintf(chosen_user, "%s", sqlite3_column_text(stmt, 2));
                if(i==nr_ip&&strcmp(sqlite3_column_text(stmt,1),"off")==0) Close=1; 
                if(i==nr_ip)break;
                
            }
            //printf("User: |%s|, IP = %s", chosen_user, IP2);
            mlen=strlen(message);
            sqlite3_finalize(stmt);
            sqlite3_close(db);

            
            if(Close==1)
            {
                char msg[]="Error: this IP is unreachable\n";
                int length=strlen(msg);
                if (write (tdL.cl, &length, sizeof(int)) <= 0)
		        {
		         printf("[Thread %d] ",tdL.idThread);
		         perror ("[Thread]Eroare la write() catre client.\n");
		        }
	            else
		            printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        
                if (write (tdL.cl, msg, length) <= 0)
		        {
		         printf("[Thread %d] ",tdL.idThread);
		         perror ("[Thread]Eroare la write() catre client.\n");
		        }
                printf("%s\n",msg);
            }
            else{
                char msg[110];
                sprintf(msg,"Closing %s\n", IP2);
                int length=strlen(msg);
                if (write (tdL.cl, &length, sizeof(int)) <= 0)
		        {
		         printf("[Thread %d] ",tdL.idThread);
		         perror ("[Thread]Eroare la write() catre client.\n");
		        }
	            else
		            printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        
                if (write (tdL.cl, msg, length) <= 0)
		        {
		         printf("[Thread %d] ",tdL.idThread);
		         perror ("[Thread]Eroare la write() catre client.\n");
		        }
                char syscom[1000];
                sprintf(syscom, "ssh -t %s@%s 'sudo shutdown -h 0'", chosen_user, IP2);
                printf("\n\n%s\n\n", syscom);
                system(syscom);

                printf("%s\n",msg);
            }
            
            

        }

        if(strcmp(comanda, "3")==0){

            //Update_cts();
            sqlite3 *db;
            char *err_msg = 0;
            sqlite3_stmt* stmt;
            
            if (sqlite3_open ("connection.db", &db) != SQLITE_OK) {
                printf("Error opening database.\n");
                return ;
            }
            
            char *sql = "SELECT * FROM Conn;";
                
            int rc= sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            if(rc!=SQLITE_OK){
                printf("Error fetching data");
                sqlite3_close(db);
            }
            char row[1000];
            while(sqlite3_step(stmt)==SQLITE_ROW){

                sprintf(row,"%s\n", sqlite3_column_text(stmt,0));
                strcat(message, row);
            }
            mlen=strlen(message);
            sqlite3_finalize(stmt);
            sqlite3_close(db);

            if (write (tdL.cl, &mlen, sizeof(int)) <= 0)
		    {
		     printf("[Thread %d] ",tdL.idThread);
		     perror ("[Thread]Eroare la write() catre client.\n");
		    }
	        else
		        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
    
            if (write (tdL.cl, message, mlen) <= 0)
		    {
		     printf("[Thread %d] ",tdL.idThread);
		     perror ("[Thread]Eroare la write() catre client.\n");
		    }
	        else
		        printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);

            int nr_ip;
            if (read (tdL.cl, &nr_ip, sizeof(int)) <= 0)
			    {
			      printf("[Thread %d]\n",tdL.idThread);
			      perror ("Eroare la READ() de la client.\n");
			    
			    }
            printf("%d\n", nr_ip);

            //Update_cts();
            
            if (sqlite3_open ("connection.db", &db) != SQLITE_OK) {
                printf("Error opening database.\n");
                return ;
            }
            
            sql = "SELECT * FROM Conn;";
                
            rc= sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
            if(rc!=SQLITE_OK){
                printf("Error fetching data");
                sqlite3_close(db);
            }
            int i=0, Close=0;
            char chosenIP[]="Couldn't close this PC";
            char IP2[100], chosen_user[10];
            while(sqlite3_step(stmt)==SQLITE_ROW){
                i++;
                    sprintf(chosenIP,"Closing %s \n", sqlite3_column_text(stmt,0)); 
                    sprintf(IP2,"%s", sqlite3_column_text(stmt,0));
                    sprintf(chosen_user, "%s", sqlite3_column_text(stmt, 2));
                if(i==nr_ip&&strcmp(sqlite3_column_text(stmt,1),"off")==0) Close=1;               
                if(i==nr_ip)break;
                
            }
            printf("User: |%s|, IP = %s", chosen_user, IP2);
            mlen=strlen(message);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            int L;
            char input[1000];
            if (read (tdL.cl, &L , sizeof(int)) <= 0)
			    {
			      printf("[Thread %d]\n",tdL.idThread);
			      perror ("Eroare la read() de la client.\n");
			    
			    }
	        if (read (tdL.cl, input, L) <= 0)
			        {
			          printf("[Thread %d]\n",tdL.idThread);
			          perror ("Eroare la read() de la client.\n");
			        
			        }
    
            if(Close==1)
            {
                char msg[]="Error: this IP is unreachable\n";
                int length=strlen(msg);
                if (write (tdL.cl, &length, sizeof(int)) <= 0)
		        {
		         printf("[Thread %d] ",tdL.idThread);
		         perror ("[Thread]Eroare la write() catre client.\n");
		        }
	            else
		            printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        
                if (write (tdL.cl, msg, length) <= 0)
		        {
		         printf("[Thread %d] ",tdL.idThread);
		         perror ("[Thread]Eroare la write() catre client.\n");
		        }
            }
            else{
                FILE *com;
                char path[1035], output[100000]="";

                char INPUT[10000];
                sprintf(INPUT,"ssh -t %s@%s '%s' ",chosen_user,IP2,input);
                printf("\n%s\n", INPUT);

                /* Open the command for reading. */
                com = popen(INPUT, "r");
                if (com == NULL) 
                {
                printf("Failed to run command\n" );    
                exit(1);
                }

                /* Read the output a line at a time - output it. */
                while (fgets(path, sizeof(path), com) != NULL) {
                    //printf("%s", path);
                    strcat(output, path);
                }
                int len=strlen(output);
                printf("Lungime %d\n Output: %s", len, output);
                    
                /* close */
                pclose(com);

                if (write (tdL.cl, &len, sizeof(int)) <= 0)
		        {
		         printf("[Thread %d] ",tdL.idThread);
		         perror ("[Thread]Eroare la write() catre client.\n");
		        }
	            else
		            printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
        
                if (write (tdL.cl, output, len) <= 0)
		        {
		         printf("[Thread %d] ",tdL.idThread);
		         perror ("[Thread]Eroare la write() catre client.\n");
		        }
	            else
		            printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",tdL.idThread);
            }

        }
            if(strcmp(comanda, "4")==0)
            {return;}
        }

}

