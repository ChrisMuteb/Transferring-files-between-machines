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
void init(int argc, char *argv[]);
int authenticateUSER(char *pswdFile, char u[20], char p[20]);
void listDirectories();
void deleteFile(char *delF);
void trimRecvbuf(char *s);
int fileNumLine(char *fl);

char u_name[20];//stores the username

int client, port_num;
int is_authen = 0;//set to 1 when user log in with USER
char p_file[20], d_file[20];
//int acc = 0;//number of line in the password.txt

char passwordFile[20] = "password.txt";

int main(int argc, char *argv[]){

    int sd,opt,optval;
    struct sockaddr_in addr;


    init(argc, argv);//get the 3 arguments and return the port number
    printf("running dir: %s\n", d_file);
    printf("Port num: %d\n", port_num);
    printf("Password: %s\n", p_file);
    //printf("I am out");
    //printf("username: %s\n", u_name);

    if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
        PANIC("Socket");
    addr.sin_family = AF_INET;

    if ( port_num > 0 )
                addr.sin_port = htons(port_num);
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
            pthread_detach(child);
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

void init(int argc, char *argv[]){

    if(argc < 6){
        error_message();
    }
    
    int optionInput;//Getopt var. holder
    
    
    while((optionInput = getopt(argc, argv, "d:p:u:")) != -1){//prompt the user for the input and output file
        
        switch (optionInput)
        {
        case 'd':
            strcpy(d_file, optarg);
            break;
        case 'p':
            port_num = atoi(optarg);
            break;
        case 'u':
            strcpy(p_file, optarg);
            break;
        default:
            error_message();
            break;
        }
    }
}

void listDirectories(){
    
    DIR *d;
    struct dirent *dir;
    d = opendir(d_file);//opendir(".");
    int bytes_r = 0, len;
    char str[DEFAULT_BUFLEN] = {0}, lenStr[5];

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s %ld\n", dir->d_name, sizeof(dir->d_name));
            //----------------
            strcpy(str, dir->d_name);
            strcat(str, "\n");
            //len = sizeof(dir->d_name);
            //display the size also
            //strcat(str, lenStr);
            bytes_r = strlen(str);
            if(bytes_r > 0)
                send(client, str, bytes_r, 0);
            
            
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
{   
    char line[DEFAULT_BUFLEN] = {0};
    int bytes_read;
    client = *(int *)arg;
    char cmd_USER[20] = {0}, cmd_uname[20] = {0}, cmd_pswd[20] = {0}, cmd_LIST[20] = {0};
    char str_USER[70] = {0};
    //-----------Welcoming message sent to user-------------
    char str[35] = {0};
    int bytes_r = 35;
    strcat(str, "Welcome to Chris\'s file server\n");
    send(client, str, bytes_r, 0);
    //-----------LIST---------------
    int rcnt = 0;
    char recvbuf[DEFAULT_BUFLEN] = {0}; 
    int recvbuflen = DEFAULT_BUFLEN;
    //-----------DEL---------------
    int rcnt_DEL = 0;
    char recvbuf_DEL[DEFAULT_BUFLEN] = {0}; 
    int recvbuflen_DEL = DEFAULT_BUFLEN;
    char del[recvbuflen];
    char delStr[recvbuflen];
    
    do
    {
        bytes_read = recv(client, line, sizeof(line), 0);//Reads USER username userpassword
        if (bytes_read > 0 && is_authen == 0) {
            
            strcpy(cmd_USER, strtok(line, " "));
            strcpy(cmd_uname, strtok(NULL, " "));
            strcpy(cmd_pswd, strtok(NULL, "\n"));
           
            if(strcmp(cmd_USER, "USER") == 0){
                printf("%s\n", cmd_uname);
                //call function that search through the password txt
                if(authenticateUSER(p_file, cmd_uname, cmd_pswd) == 1){

                    char str_USER[40] = {0};
                    strcpy(str_USER, "\n200 User ");
                    strcat(str_USER, cmd_uname);
                    strcat(str_USER, " granted to access.\n");
                    is_authen = 1;//set to true

                    if(bytes_r <= 0)
                    bytes_r = 35;
    
                    while(bytes_r > 0){
                        send(client, str_USER, bytes_r, 0);
                        break;
                    }
                    
                }else{
                    
                    strcpy(str_USER, "\n400 User ");
                    strcat(str_USER, cmd_uname);
                    strcat(str_USER, " not found. Please try with another user\n");
                   
                    bytes_r = 70;
    
                    while(bytes_r > 0){
                        send(client, str_USER, bytes_r, 0);
                        break;
                    }
                }
            }
                if ((bytes_r=send(client, str_USER, bytes_r, 0)) < 0 ) {
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
        
        
        //-------------------------
        rcnt = recv(client, recvbuf, recvbuflen, 0);
        
        //bytes_read = rcnt;
        if(is_authen == 1 && recvbuf[0] == 'L'){
            
            char lst[recvbuflen];
            strcpy(lst,"LIST");
            trimRecvbuf(recvbuf);
            
            int iseq = 0;
            for(int i = 0; i < 4; i++){
                if(recvbuf[i] == lst[i])
                    iseq = 1;
                else 
                    iseq = 0;
            }
           //printf("\n%d\n", iseq);
            if (rcnt > 0 && (iseq == 1)) {//user entered LIST
                listDirectories();
                is_authen = 1;
                printf("Bytes received: %d\n", rcnt);
            // Echo the buffer back to the sender
            strcat(recvbuf, "\n");
            rcnt = send( client, recvbuf, rcnt, 0 );
            if (rcnt < 0) {
                printf("Send failed:\n");
                close(client);
                break;
            }
            printf("Bytes sent: %d\n", rcnt);

        }
        }
        
        rcnt_DEL = recv(client, recvbuf_DEL, recvbuflen_DEL, 0);
        //bytes_read = rcnt_DEL;
        if(is_authen == 1 && recvbuf_DEL[0] == 'D'){
            printf("Let's delete some files");
                
            is_authen = 1;
            //strcpy(lst,"LIST");
            strcpy(del, strtok(recvbuf_DEL, " "));
            strcpy(delStr, strtok(NULL, " "));
            printf("\n%s%s", del, delStr);
            trimRecvbuf(recvbuf);
            
            int iseq = 0;
            if(strcmp(del, "DEL") == 0){

                printf("We are now ready to delete\n");
                deleteFile(delStr);

            }
            /*for(int i = 0; i < 4; i++){
                if(recvbuf[i] == lst[i])
                    iseq = 1;
                else 
                    iseq = 0;
            }*/
           //printf("\n%d\n", iseq);
            if (rcnt > 0 && (iseq == 1)) {//user entered LIST
                listDirectories();
                printf("Bytes received: %d\n", rcnt);
            // Echo the buffer back to the sender
            strcat(recvbuf, "\n");
            rcnt = send( client, recvbuf, rcnt, 0 );
            bytes_read = rcnt;
            if (rcnt < 0) {
                printf("Send failed:\n");
                close(client);
                break;
            }
            printf("Bytes sent: %d\n", rcnt);

        }
        }
        
        //-----------------------------
    } while (bytes_read > 0);
    close(client);
    return arg;
}
void trimRecvbuf(char *s){
    int i = strlen(s) - 1;
    while(i > 0){
        if(s[i] == '\a' || s[i] == '\b' || s[i] == '\f' || s[i] == '\v' || s[i] == ' ' || s[i] == '\n' || s[i] == '\t' || s[i] == '\r') i--;
        else break;
    }
    s[i+1] = '\0';
}


void deleteFile(char *delF){

    DIR *d;
    struct dirent *dir;
    d = opendir(d_file);//opendir(".");
    int bytes_r = 0, len;
    char str[DEFAULT_BUFLEN] = {0}, lenStr[5];
    int acc = 0;

    is_authen = 1;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            //printf("%s %ld\n", dir->d_name, sizeof(dir->d_name));
            //----------------
            //strcpy(str, dir->d_name);
            trimRecvbuf(delF);
            printf("\ndname: %s %s %ld %ld\n", dir->d_name, delF, strlen(dir->d_name), strlen(delF));
            if(strcmp(dir->d_name, delF) == 0){
                //if (unlink(delF)) perror("unlink");
                char file[500];
                char str_sms_del[500] = {0};
                int str_sms = 0;
                strcpy(file, d_file);
                strcat(file,"/");
                strcat(file, delF);
                printf("%s\n", file);

                if (remove(file) == 0) {
                    printf("The file is deleted successfully.");
                    strcpy(str_sms_del, "200 File ");
                    strcat(str_sms_del, delF);
                    strcat(str_sms_del, " deleted.\n");
                    str_sms = 50;
                    if(str_sms > 0)
                        send(client, str_sms_del, str_sms,0);
                    
                }else {
                    printf("The file is not deleted.");
                    //
                }
                printf("we are going to delete");
                break;
            }else{
                acc++;
                
            }
        }
       
        if(acc == fileNumLine(d_file)){
                printf("FIle not found!");
                char dname[50] = {0};
                char str_d[50] = {0};
                int dname_s = 0;
                strcpy(dname, delF);
                trimRecvbuf(dname);
                strcpy(str_d, "404 File ");
                strcat(str_d, dname);
                strcat(str_d, " is not on the server.\n");
                
                dname_s = 50;
                if(dname_s > 0)
                    send(client, str_d, dname_s, 0);
            }

        closedir(d);
    }
}

int fileNumLine(char *fl){
    DIR *d;
    struct dirent *dir;
    d = opendir(d_file);//opendir(".");
    int acc = 0;
    

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            //printf("\ndname: %s %s %ld %ld\n", dir->d_name, delF, strlen(dir->d_name), strlen(delF));
            acc++;
        }
    }
    return acc;
    
}