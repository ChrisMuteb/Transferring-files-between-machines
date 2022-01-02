#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void error_message();
int password_correct(char *pswdFile, char *pswd);
void init(int argc, char *argv[]);

int main(int argc, char *argv[]){

    init(argc, argv);
    

    exit(EXIT_SUCCESS);
}
void error_message(){
    printf("Required: -d <directory> | -p <port number> | -u <password>\n");
    exit(2);
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
            puts("Passwd found");
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
    if(argc < 6){//ensures that the number of arguments is not less than 4
        error_message();
    }
    
    int optionInput;//Getopt var. holder
    char *directory, *port_num, *password;
    
    
    while((optionInput = getopt(argc, argv, "d:p:u:")) != -1){//prompt the user for the input and output file
        
        switch (optionInput)
        {
        case 'd':
            directory = optarg;
            break;
        case 'p':
            port_num = optarg;
            break;
        case 'u':
            password = optarg;
            break;
        default:
            error_message();
            break;
        }
        
    }
    printf("running dir: %s\n", directory);

    printf("port num: %s\n", port_num);
    printf("password: %s\n", password);

    puts("");
    if(password_correct("password.txt", password)){
        puts("password found!");
    }
}