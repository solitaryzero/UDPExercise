#ifndef COMMON_H
#define COMMON_H

#define SERVERIPADDR "127.0.0.1"
#define SERVERPORT 8001
#define MAXBUFSIZE 4096
#define HEADERLEN (int)(sizeof(int)*3+sizeof(char))
#define MAXSTRINGLEN (MAXBUFSIZE-HEADERLEN-1)
#define MAXSEQNUM 10000
#define MAXRETRY 3
#define MAXWAITTIME 3
#define MAXREQPERSEC 50
#define MAXREQGLOB 500

#endif