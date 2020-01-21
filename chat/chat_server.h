
typedef struct PlayerNode
{
    int data;
    struct PlayerNode* next;
    struct PlayerNode* prev;
    char ip[16];
    char name[31];
}PlayerList;

PlayerList *newNode(int socket, char* ip){
    PlayerList *np = (PlayerList *)malloc(sizeof(PlayerList));
    np->data = socket;
    np->next = NULL;
    np->prev = NULL;
    strncpy(np->ip,ip,16);
    strncpy(np->name,"NULL",5);
    return np;
}
