#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void error_message();

int main(int argc, char *argv[]){

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
    printf("running dir: %s\n", port_num);
    printf("running dir: %s\n", password);
    
    exit(EXIT_SUCCESS);
}
void error_message(){
    printf("Required: -d <directory> | -p <port number> | -u <password>\n");
    exit(2);
}