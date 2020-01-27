//プレイヤーに関する構造体
typedef struct PlayerNode
{
    int data;
    struct PlayerNode* next;
    struct PlayerNode* prev;
    char ip[16];
    char name[31];
    struct RoomNode* room;
}PlayerList;


//ルームに関する構造体
typedef struct RoomNode
{
    int id;
    struct RoomNode* next;
    struct RoomNode* prev;    
}RoomList;


//プレイヤーのノードを作る
PlayerList *newNode(int socket, char* ip){
    PlayerList *np = (PlayerList *)malloc(sizeof(PlayerList));
    np->data = socket;
    np->next = NULL;
    np->prev = NULL;
    strncpy(np->ip,ip,16);
    strncpy(np->name,"NULL",5);
    return np;
}


//ルームのノードを作る
RoomList *newRoomNode(){
    RoomList *np = (RoomList *)malloc(sizeof(RoomList));
    np->id = -1;
    np->next = NULL;
    np->prev = NULL;
    return np;
}