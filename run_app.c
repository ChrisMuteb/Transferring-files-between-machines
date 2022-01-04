/* echo_server_thread.c
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <dirent.h>

/*Definition*/
#define DEFAULT_BUFLEN 1024
#define PORT 8200

void PANIC(char* msg);
#define PANIC(msg)  { perror(msg); exit(-1); }
void* Child(void* arg);

//----------------------------------------------
void error_message();
int password_correct(char *pswdFile, char *pswd);
void init(int argc, char *argv[], int *port_num);
int authenticateUSER(char *pswdFile, char u[20], char p[20]);
void listDirectories();

char u_name[20];//stores the username
char passwordFile[20] = "password.txt";//file containing password

int main(int argc, char *argv[]){

    int sd,opt,optval;
    struct sockaddr_in addr;
    //unsigned short port=0;
    int p_num = 0;//port num from the cmd argument


    init(argc, argv, &p_num);//get the 3 arguments and return the port number
    //printf("I am out");
    //printf("username: %s\n", u_name);

    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        PANIC("Socket");
    addr.sin_family = AF_INET;

    if ( p_num > 0 )
                addr.sin_port = htons(p_num);
    else
                addr.sin_port = htons(PORT);

    addr.sin_addr.s_addr = INADDR_ANY;

   // set SO_REUSEADDR on a socket to true (1):
   optval = 1;
   setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);


    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
        PANIC("Bind");
    if ( listen(sd, SOMAXCONN) != 0 )
        PANIC("Listen");

    printf("File server listening on localhost port %d\n",ntohs(addr.sin_port));

    while (1)
    {
        int client, addr_size = sizeof(addr);
        pthread_t child;

        client = accept(sd, (struct sockaddr*)&addr, &addr_size);
        printf("Connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        if ( pthread_create(&child, NULL, Child, &client) != 0 )
            perror("Thread creation");
        else
            pthread_detach(child);  /* disassociate from parent */
    }

    

    exit(EXIT_SUCCESS);
}
void error_message(){
    printf("Required: -d <directory> | -p <port number> | -u <password>\n");
    exit(1);
}
int password_correct(char *pswdFile, char *pswd){

    FILE *spin;
    int numIn;
    char in[20];
    char username[20], passwd[20];
    int ispswdFound = 0;
    
    spin = fopen(pswdFile, "r");
    if(!spin){
        printf("Could not open file\n");
        exit(1);
    }
    

    //printf("%d\n", strlen())
    while(fgets(in, 20, spin)){

        strcpy(username, strtok(in, ":"));
        strcpy(passwd, strtok(NULL, "\n"));

        if(strcmp(pswd, passwd) == 0){
            //puts("Passwd found");
            strcpy(u_name, username);
            ispswdFound = 1;
            break;
        }
    }
    if(ispswdFound == 0){
        puts("Wrong password!\nApp will close shortly");
        exit(1);
    }
    return ispswdFound;
}

void init(int argc, char *argv[], int *port_num){
    if(argc < 6){//ensures that the number of arguments is not less than 4
        error_message();
    }
    
    int optionInput;//Getopt var. holder
    char *directory, *password;
    
    
    while((optionInput = getopt(argc, argv, "d:p:u:")) != -1){//prompt the user for the input and output file
        
        switch (optionInput)
        {
        case 'd':
            directory = optarg;
            break;
        case 'p':
            *port_num = atoi(optarg);
            break;
        case 'u':
            password = optarg;
            break;
        default:
            error_message();
            break;
        }
    
    }
    //printf("running dir: %s\n", directory);

    //printf("Port num: %d\n", *port_num);
    //printf("Password: %s\n", password);

    puts("");
    if(password_correct("password.txt", password)){
        //puts("Your password is correct!");
        puts("");
    }

}

void listDirectories(){
    
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
        }

        closedir(d);
    }

  
}

int authenticateUSER(char *pswdFile, char u[20], char p[20]){
    FILE *spin;
    int numIn;
    char in[20];
    char username[20], passwd[20];
    int ispswdFound = 0;
    
    spin = fopen(pswdFile, "r");
    if(!spin){
        printf("Could not open file\n");
        exit(1);
    }
    

    //printf("%d\n", strlen())
    while(fgets(in, 20, spin)){

        strcpy(username, strtok(in, ":"));
        strcpy(passwd, strtok(NULL, "\n"));

        if((strcmp(p, passwd) == 0) && (strcmp(u, username) == 0)){
            //puts("Passwd found");
            //strcpy(u_name, username);
            ispswdFound = 1;
            break;
        }
    }
    return ispswdFound;
}
/*--------------------------------------------------------------------*/
/*--- Child - echo server                                         ---*/
/*--------------------------------------------------------------------*/
void* Child(void* arg)
{   char line[DEFAULT_BUFLEN];
    int bytes_read;
    int client = *(int *)arg;
    //-----------Welcoming message sent to user-------------
    char str[35] = {0};
    int bytes_r = 0;
    strcat(str, "Welcome to ");
    strcat(str, u_name);
    strcat(str, "\'s file server. \n");
    if(bytes_r <= 0)
        bytes_r = 35;
    
   while(bytes_r > 0){
       send(client, str, bytes_r, 0);
       break;
   }
   puts("\n");
    //--------------------------
    //strcpy(username, strtok(in, ":"));
    //strcpy(passwd, strtok(NULL, "\n"));
    char cmd_USER[20] = {0}, cmd_uname[20] = {0}, cmd_pswd[20] = {0};
    do
    {
        bytes_read = recv(client, line, sizeof(line), 0);
        if (bytes_read > 0) {
            
            strcpy(cmd_USER, strtok(line, " "));
            strcpy(cmd_uname, strtok(NULL, " "));
            strcpy(cmd_pswd, strtok(NULL, "\n"));
           
            if(strcmp(cmd_USER, "USER") == 0){
                printf("%s\n", cmd_uname);
                //call function that search through the password txt
                puts("");
                if(authenticateUSER(passwordFile, cmd_uname, cmd_pswd) == 1){
                    char str_USER[40] = {0};
                    strcpy(str_USER, "\n200 User ");
                    strcat(str_USER, cmd_uname);
                    strcat(str_USER, " granted to access.\n");
                    //printf("%s\n", cmd_USER);
                    //send(client, str, bytes_read, 0);
                    if(bytes_r <= 0)
                    bytes_r = 35;
    
                    while(bytes_r > 0){
                        send(client, str_USER, bytes_r, 0);
                        break;
                    }
                    puts("\n");
                    //check for listing files
                }else{
                    char str_USER[70] = {0};
                    strcpy(str_USER, "\n400 User ");
                    strcat(str_USER, cmd_uname);
                    strcat(str_USER, " not found. Please try with another user\n");
                    //printf("%s\n", cmd_USER);
                    //send(client, str, bytes_read, 0);
                    //if(bytes_r <= 0)
                    bytes_r = 70;
    
                    while(bytes_r > 0){
                        send(client, str_USER, bytes_r, 0);
                        break;
                    }
                    puts("\n");
                }
            }
                if ((bytes_read=send(client, line, bytes_read, 0)) < 0 ) {
                        printf("Send failed\n");
                        break;
                }
        } else if (bytes_read == 0 ) {
                printf("Connection closed by client\n");
                break;
        } else {
                printf("Connection has problem\n");
                break;
        }
    } while (bytes_read > 0);
    close(client);
    return arg;
}



