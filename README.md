# Transferring files between machines
Implementation of a modified version of the File Transfer Protocol (FTP) for transferring files between two machines: Socket Programming

## How To run the program

To begin, You will need at least two CLIs to simulate the server(runs the program) and the client(request data from server)
  * On the server:
      - Compile by typing: "gcc run_app.c -o run_app -lpthread"
      - Run by typing: "./run_app -d filepath -p portNumber -u passwordFile
      - The the server will be listening on the port number mentioned above.
  * On the client side:
      - A welcoming message will be displayed: "Welcome to Chris' file server."
      - Enter: "USER <username> <userpassword>" the server will ensure that only the name and corresponding password are located in the passwordFile.
              $ If username and password are correct "200 User <username> to access" is displayed
              $ Otherwise, "400 User not found." will be shown.
      - Enter: "LIST" will list all the files from the directory file specified at the begining. The file size was not sent to the client but display on the server side.
      - Enter: "DEL <filename>" will delete the file. 
              $ if file is found in the folder, "200 File <filename> deleted."
              $ Otherwise, "404 File <filename> is not on the server."
      - Enter: "QUIT" will send a "Goodbye!" message to the client.
  
 These screenshots illustrate the 2 different clients accessing the server and running the command as mentioned above.
![anotherUser](https://user-images.githubusercontent.com/31598690/148259478-b9b91537-6759-4288-a56a-635a1edf822b.png)
![screenShot](https://user-images.githubusercontent.com/31598690/148259501-07b020e8-7797-4139-a6ab-736612305c7a.png)
