#include<stdio.h>
#include<string.h>    // for strlen
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> // for inet_addr
#include<unistd.h>    // for write
#include<pthread.h>   // for threading, link with lpthread
//It has been assumed that there will be a maximum of 10 rooms in the program.
//It is assumed that there will be a maximum of 10 users in a room.
// If socketNo -1 in Userstruct, it is defined as no user.
// If room no is less than 0 in the user struct, it means it has no room.
// The index in the room is kept, thus making it easier to send messages.
// The nicname, roomname and password are assumed to be a maximum of 100 characters
// struct where user information is kept
struct UserStruct {
    int socketNo;
    char NickName[100];
    char RoomName[100];
    int oda_no;
    int sira_no;
};
typedef struct UserStruct UserStruct;
//Room struct
struct RoomStruct {
    UserStruct Users[10];
    char RoomName[100];
    char Password[100];
    int private;
    int number_of_users;
};
typedef struct RoomStruct RoomStruct;
//Global variables
RoomStruct Rooms[10];
int number_of_rooms = 0;
//To reset information when rooms are empty
void Room_Reset(int i){
        Rooms[i].number_of_users = 0;
        Rooms[i].private = -1;
        strcpy(Rooms[i].RoomName,"");
        strcpy(Rooms[i].Password,"");
        for(int j=0;j<10;j++){
            strcpy(Rooms[i].Users[j].NickName,"");
            strcpy(Rooms[i].Users[j].RoomName,"");
            Rooms[i].Users[j].socketNo = -1;
            Rooms[i].Users[j].sira_no= -1;
        }
}
//List function
void List(int socketnumber){
    if(number_of_rooms > 0){
        char ListUser[20000] = "Rooms :\n ";
        for(int i = 0;i < 10; i++){
            if(Rooms[i].number_of_users > 0){
                strcat(ListUser,Rooms[i].RoomName);
                strcat(ListUser," : ");
                //If the "private" variable is -1, the room is not hidden. If 0, the room is hidden. The user names inside are printed
                if(Rooms[i].private == -1){
                    for(int j = 0; j<10;j++){
                        if(Rooms[i].Users[j].socketNo != -1){
                            strcat(ListUser,Rooms[i].Users[j].NickName);
                            strcat(ListUser,",");
                        }
                    }
                }
                else{
                    strcat(ListUser,"Private room...");
                }
                strcat(ListUser,"\n");
            }
        }
        send(socketnumber , ListUser , strlen(ListUser),0);
    }
    else{
        char message[200] = "No rooms have been created yet...\n";
        send(socketnumber , message , strlen(message),0);
    }
}
//room creation function
int create(UserStruct *user,char* Room_Name,char* komut){
    //Control is made as it is assumed that a maximum of 10 rooms will be used
    if(number_of_rooms < 10){
        //the room name entered is checked to see if it is unique
        int is_exist = 0;
        for(int i = 0;i < 10; i++){
            if(Rooms[i].number_of_users != 0 && strcmp(Room_Name,Rooms[i].RoomName) == 0){
                is_exist = 1;
                break;
            }
        }
        if(is_exist == 1){
            char *msg = "This room name has already used...\n";
            send(user->socketNo , msg , strlen(msg),0);
        }
        else{
            for(int i = 0;i < 10; i++){
                //If the room name is unique, an empty index is searched in the rooms array.
                if(Rooms[i].number_of_users == 0){
                    Rooms[i].number_of_users++;
                    strcpy(user->RoomName,Room_Name);
                    strcpy(Rooms[i].RoomName,Room_Name);
                    user->oda_no= i;
                    user->sira_no=0;
                    Rooms[i].Users[0] = *user;
                    //pcreate
                    if(strcmp("-pcreate",komut)==0){
                        char message[100]; 
                        strcpy(message, "Please enter room password...");
                        send(user->socketNo , message , strlen(message),0);
                        recv(user->socketNo,Rooms[i].Password,100,0);
                        Rooms[i].private = 0;
                    }
                    char *msg = "Room created...\n";
                    send(user->socketNo , msg , strlen(msg),0);
                    number_of_rooms++;
                    break;
                }
            }
        }
    }
    else{
        char *msg = "Cannot open rooms because maximum number of rooms has been reached...\n";
        send(user->socketNo , msg , strlen(msg),0);
    }
}

void Message(UserStruct *user,char* message){
    char msg[200];
    strcpy(msg,user->NickName);
    strcat(msg," : ");
    strcat(msg,message);
    for(int i=0;i<10;i++){
        //Since the user is kept in which room, a message is sent one by one to the users in that room.
        if(Rooms[user->oda_no].Users[i].socketNo != user->socketNo && Rooms[user->oda_no].Users[i].socketNo != -1){
            send(Rooms[user->oda_no].Users[i].socketNo , msg , strlen(msg),0);
        }
    }
}

void Enter_Room(UserStruct *user,char* Room_Name){
    int found = -1;
    int entered = -1;
    for(int i = 0;i < 10; i++){
        if(strcmp(Room_Name,Rooms[i].RoomName) == 0){
            found = 1;
            //If the room is found, it is checked whether it is full or not
            if( Rooms[i].number_of_users < 10){
                if(Rooms[i].private == 0){
                    char message[100]; 
                    char temp[100];
                    strcpy(message, "This room is private. Please enter room password");
                    send(user->socketNo , message , strlen(message),0);
                    recv(user->socketNo,temp,100,0);
                    if(strcmp(Rooms[i].Password,temp) == 0){
                        entered = 0;
                    }
                    else{
                        strcpy(message, "Password incorrect...");
                        send(user->socketNo , message , strlen(message),0);
                    }
                }
                else{
                    entered = 0;
                }
                if(entered == 0){
                    for(int j = 0;j < 10; j++){
                        if(Rooms[i].Users[j].socketNo == -1){
                            user->oda_no= i;
                            user->sira_no=j;
                            strcpy(user->RoomName,Room_Name);
                            Rooms[i].number_of_users++;
                            Rooms[i].Users[j] = *user;
                            char message[100]; 
                            strcpy(message, "You the entered room...");
                            send(user->socketNo , message , strlen(message),0);
                            break;
                        }
                    }
                }
            }
            else{
                char *msg = "Room is full...\n";
                send(user->socketNo , msg , strlen(msg),0);
                break;
            }
        }
    }
    if(found == -1){
        char *msg = "No such a room name...\n";
        send(user->socketNo , msg , strlen(msg),0);
    }
}
//User function
void *connection_handler(void *socket_desc)
{
    int sock = *((int*)socket_desc);   
    UserStruct user;
    user.socketNo = sock;
    user.oda_no = -1;
    char message[200] = "Please enter your nickname :";
    send(sock , message , strlen(message),0);
    recv(sock,user.NickName,100,0);
    //The cycle continues until the socket is turned off
    while (sock != -1)
    {
        char msg[100]="";
        char temp[100]="";
        char *temp1;
        recv(sock,msg,100,0);
        puts(msg);
        strcpy(temp,msg);
        char *token = (char*)malloc(100*sizeof(char));
        if(strcmp(msg,"-list") == 0){
            if(user.oda_no < 0){
                List(sock);
            }
            else{
                strcpy(message, "You have already entered the room. You cannot run the -list function");
                send(sock , message , strlen(message),0);
            }
        }
        else if(strcmp(msg,"-whoami") == 0){
            send(sock , user.NickName , strlen(user.NickName),0);
        }
        else if(strcmp(msg,"-exit") == 0){
            //If the exit function is entered and there is no room, exit is provided. Socket number is reset
            if(user.oda_no < 0){
                sock = -1;
                strcpy(message,"You exit the program...\n");
                free(socket_desc);
                send(user.socketNo , message , strlen(message),0);
            }
            else{
                strcpy(message,"You are in the room, so you cannot exit the system...\n");
                send(sock , message , strlen(message),0);
            }
        }
        token = strtok(temp," ");
        if((strcmp(token,"-create")==0 && strlen(msg) >7 )|| (strcmp(token,"-pcreate")==0 &&strlen(msg) >8)){
            if(user.oda_no == -1){
                //If the user does not have a room, it is sent to the room creation function
                temp1 = strstr(msg," ");
                create(&user,temp1,token);
            }
            else{
                strcpy(message,"You have already joined the room... You cannot create another room.\n");
                send(sock , message , strlen(message),0);
            }
        }
        else if((strcmp(token,"-create")==0) || (strcmp(token,"-pcreate")==0)){
                //If the room name is not entered, the room name is requested.
                strcpy(message,"Please enter room name...\n");
                send(sock , message , strlen(message),0);
        }
        else if(strcmp(token,"-enter")==0){
            if(strlen(msg)> 6){
                //If the room name is entered, it is sent to the enter function.
                if(user.oda_no == -1){
                    temp1 = strstr(msg," ");
                    Enter_Room(&user,temp1);
                }
                else{
                    //If the user has a room, the message is sent.
                    strcpy(message,"You have already joined the room...\n");
                    send(sock , message , strlen(message),0);
                }
            }
            else
            {
                //If the room name is not entered, the room name is requested
                strcpy(message,"Please enter room name......\n");
                send(sock , message , strlen(message),0);
            }
            
        }
        else if(strcmp(token,"-msg")==0){
            if(user.oda_no > -1){
                //If no message is entered, a message is requested.
                if(strlen(msg)> 4){
                    temp1 = strstr(msg," ");
                    Message(&user,temp1);
                }
                else{
                    strcpy(message,"Please enter your message\n");
                    send(sock , message , strlen(message),0);
                }
            }
            else{
                //If it does not have a room, an error message is sent
                strcpy(message,"You haven't joined the room...\n");
                send(sock , message , strlen(message),0);
            }
        }

        else if(strcmp(token,"-quit")==0){
            //The function works if the room name is entered
            if(strlen(msg)> 5){
                token = strstr(msg," ");
                if(user.oda_no == -1){
                    //A message is sent if the user does not have a room
                    strcpy(message, "You haven't joined the room\n");
                    send(sock , message , strlen(message),0);
                }
                else if(strcmp(token,user.RoomName) == 0){
                    //If the user has a room and the roomname is entered correctly, the information of the user in the room is reset.
                    UserStruct temp;
                    Rooms[user.oda_no].number_of_users--;
                    //If there are no users left in the room, the room is closed.
                    if(Rooms[user.oda_no].number_of_users == 0){
                        Room_Reset(user.oda_no);
                        number_of_rooms--;
                    }
                    else{
                        Rooms[user.oda_no].Users[user.sira_no] = temp;
                        Rooms[user.oda_no].Users[user.sira_no].socketNo = -1;
                    }
                    user.oda_no = -1;
                    user.sira_no = -1;
                    strcpy(user.RoomName,"");
                    strcpy(message, "You quit the room\n");
                    send(sock , message , strlen(message),0);
                }
                else{
                    strcpy(message, "Wrong room name...\n");
                    send(sock , message , strlen(message),0);
                }
            }
            else{
                strcpy(message,"Please enter room name....\n");
                send(sock , message , strlen(message),0);
            }
        }
    }   
    return 0;
}

int main(int argc, char *argv[])
{
    for(int i = 0;i< 10;i++){
        Room_Reset(i);
    }  
    int socket_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;
    char *message;
     
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }
     
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(3205);
     
    if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Binding failed");
        return 1;
    }  
    listen(socket_desc, 100);
    c = sizeof(struct sockaddr_in);
    while((new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        int socket_des= new_socket;
        puts("Connection accepted");
        message = "You joined the system. WELCOME! \n";
        write(new_socket, message, strlen(message));
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;
        if(pthread_create(&sniffer_thread, NULL, connection_handler,
                          (void*)new_sock) < 0)
        {
            puts("Could not create thread");
            return 1;
        }
    }
    return 0;
}
