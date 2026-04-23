#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AF_INET 2
#define SOCK_STREAM 1
#define INADDR_ANY 0
typedef UINT_PTR SOCKET;

struct my_in_addr {
    unsigned long s_addr;
};
struct my_sockaddr_in {
    short sin_family;
    unsigned short sin_port;
    struct my_in_addr sin_addr;
    char sin_zero[8];
};
typedef struct my_WSAData {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char *lpVendorInfo;
} MY_WSADATA;

int (WINAPI *myWSAStartup)(WORD, MY_WSADATA*);
SOCKET (WINAPI *mysocket)(int, int, int);
int (WINAPI *mybind)(SOCKET, const struct my_sockaddr_in*, int);
int (WINAPI *mylisten)(SOCKET, int);
SOCKET (WINAPI *myaccept)(SOCKET, struct my_sockaddr_in*, int*);
int (WINAPI *myrecv)(SOCKET, char*, int, int);
int (WINAPI *mysend)(SOCKET, const char*, int, int);
int (WINAPI *myclosesocket)(SOCKET);
int (WINAPI *myWSACleanup)(void);
unsigned short (WINAPI *myhtons)(unsigned short);

void init_winsock() {
    HMODULE h = LoadLibrary("ws2_32.dll");
    myWSAStartup = (void*)GetProcAddress(h, "WSAStartup");
    mysocket = (void*)GetProcAddress(h, "socket");
    mybind = (void*)GetProcAddress(h, "bind");
    mylisten = (void*)GetProcAddress(h, "listen");
    myaccept = (void*)GetProcAddress(h, "accept");
    myrecv = (void*)GetProcAddress(h, "recv");
    mysend = (void*)GetProcAddress(h, "send");
    myclosesocket = (void*)GetProcAddress(h, "closesocket");
    myWSACleanup = (void*)GetProcAddress(h, "WSACleanup");
    myhtons = (void*)GetProcAddress(h, "htons");
}

int board[9][9] = {
    {5,3,0,0,7,0,0,0,0}, {6,0,0,1,9,5,0,0,0}, {0,9,8,0,0,0,0,6,0},
    {8,0,0,0,6,0,0,0,3}, {4,0,0,8,0,3,0,0,1}, {7,0,0,0,2,0,0,0,6},
    {0,6,0,0,0,0,2,8,0}, {0,0,0,4,1,9,0,0,5}, {0,0,0,0,8,0,0,7,9}
};
int original[9][9];
char html_buffer[16384];

void save_original() {
    int i, j;
    for(i=0; i<9; i++)
        for(j=0; j<9; j++)
            original[i][j] = board[i][j];
}

int is_valid(int r, int c, int num) {
    int i, j;
    for(i=0; i<9; i++) if(board[r][i] == num) return 0;
    for(i=0; i<9; i++) if(board[i][c] == num) return 0;
    int sr = (r/3)*3, sc = (c/3)*3;
    for(i=0; i<3; i++)
        for(j=0; j<3; j++)
            if(board[sr+i][sc+j] == num) return 0;
    return 1;
}

int check_win() {
    int i, j;
    for(i=0; i<9; i++)
        for(j=0; j<9; j++)
            if(board[i][j] == 0) return 0;
    return 1;
}

void format_html(char* msg) {
    int i, j;
    char temp[4096] = "";
    strcpy(html_buffer, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<!DOCTYPE html><html><head><style>"
        "body{font-family:sans-serif;text-align:center;background:#f0f2f5;margin-top:50px;}"
        "h1{color:#333;font-size:36px;}"
        "table{margin:0 auto;border-collapse:collapse;background:#fff;border:4px solid #333;box-shadow:0 4px 8px rgba(0,0,0,0.1);}"
        "td{width:60px;height:60px;border:1px solid #ccc;font-size:24px;text-align:center;}"
        "td.f{font-weight:bold;color:#111;background:#e9ecef;}"
        "td.u{color:#007bff;font-weight:bold;}"
        "td:nth-child(3n){border-right:4px solid #333;}"
        "tr:nth-child(3n) td{border-bottom:4px solid #333;}"
        "input{width:40px;height:40px;font-size:24px;text-align:center;border:none;background:transparent;color:#007bff;font-weight:bold;}"
        ".msg{color:#d9534f;font-weight:bold;margin:20px;font-size:20px;}"
        ".win{color:#5cb85c;font-weight:bold;margin:20px;font-size:24px;}"
        "button{padding:12px 24px;font-size:20px;cursor:pointer;background:#007bff;color:#fff;border:none;border-radius:4px;margin-top:20px;}"
        "button:hover{background:#0056b3;}"
        ".footer{margin-top:30px;font-size:16px;color:#777;}"
        "</style></head><body><h1>C Soduku Game Project</h1>");

    if (msg) {
        sprintf(temp, "<div class='msg'>%s</div>", msg);
        strcat(html_buffer, temp);
    }
    
    if (check_win()) {
        strcat(html_buffer, "<div class='win'>You Won!</div>");
    }

    strcat(html_buffer, "<form method='GET' action='/'>");
    strcat(html_buffer, "<table>");
    for(i=0; i<9; i++) {
        strcat(html_buffer, "<tr>");
        for(j=0; j<9; j++) {
            if(original[i][j] != 0) {
                sprintf(temp, "<td class='f'>%d</td>", original[i][j]);
                strcat(html_buffer, temp);
            } else {
                strcat(html_buffer, "<td class='u'>");
                if(board[i][j] != 0) {
                    sprintf(temp, "<input type='number' min='1' max='9' name='c%d%d' value='%d'>", i, j, board[i][j]);
                } else {
                    sprintf(temp, "<input type='number' min='1' max='9' name='c%d%d' value=''>", i, j);
                }
                strcat(html_buffer, temp);
                strcat(html_buffer, "</td>");
            }
        }
        strcat(html_buffer, "</tr>");
    }
    strcat(html_buffer, "</table>");
    strcat(html_buffer, "<button type='submit'>Submit Moves</button>");
    strcat(html_buffer, "</form><div class='footer'>Developed by Aarush Balyan</div></body></html>");
}

int main() {
    init_winsock();
    save_original();
    
    MY_WSADATA wsa;
    myWSAStartup(MAKEWORD(2, 2), &wsa);
    
    SOCKET s = mysocket(AF_INET, SOCK_STREAM, 0);
    struct my_sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = myhtons(8080);
    mybind(s, &addr, sizeof(addr));
    mylisten(s, 10);

    system("start http://localhost:8080");

    while(1) {
        SOCKET c = myaccept(s, NULL, NULL);
        if (c == ~0) continue;
        
        char buf[4096] = {0};
        myrecv(c, buf, 4095, 0);
        char* msg = NULL;

        if(strncmp(buf, "GET /?", 6) == 0) {
            char* ptr = buf + 6;
            int i, j;
            for(i=0; i<9; i++) {
                for(j=0; j<9; j++) {
                    if(original[i][j] == 0) board[i][j] = 0;
                }
            }
            while(*ptr && *ptr != ' ') {
                if(*ptr == 'c' && *(ptr+3) == '=') {
                    int r = *(ptr+1) - '0';
                    int c_idx = *(ptr+2) - '0';
                    if(r>=0 && r<=8 && c_idx>=0 && c_idx<=8) {
                        if(*(ptr+4) >= '1' && *(ptr+4) <= '9') {
                            board[r][c_idx] = *(ptr+4) - '0';
                        }
                    }
                }
                ptr++;
            }
        }
        
        char err_msg[256];
        err_msg[0] = '\0';
        int invalid = 0;
        int i, j;
        for(i=0; i<9; i++) {
            for(j=0; j<9; j++) {
                if (board[i][j] != 0 && original[i][j] == 0) {
                    int tmp = board[i][j];
                    board[i][j] = 0;
                    if(!is_valid(i, j, tmp)) {
                        invalid = 1;
                        sprintf(err_msg, "Invalid move! Number %d at Row %d, Col %d conflicts.", tmp, i+1, j+1);
                    }
                    board[i][j] = tmp;
                }
            }
        }
        if(invalid) msg = err_msg;

        format_html(msg);
        mysend(c, html_buffer, strlen(html_buffer), 0);
        myclosesocket(c);
        
        if (strncmp(buf, "GET /favicon.ico", 16) == 0) continue;
    }
    
    myclosesocket(s);
    myWSACleanup();
    return 0;
}
