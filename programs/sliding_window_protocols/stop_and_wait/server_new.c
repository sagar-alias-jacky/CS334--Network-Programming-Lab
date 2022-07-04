#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#define PORT 8080
#define MAXSZ 100
typedef enum
{
    DATA,
    ACK
} FRAMEKIND;
struct timeval timeout; //{};*/
struct Frame
{
    FRAMEKIND type;   /* DATA or ACK */
    unsigned int len; /* the length of the msg field only */
    int seq;          /* only ever 0 or 1 */
    char *msg;
};
struct Frame *makeframes()
{
    int i;
    int seqno = 1;
    struct Frame *fr = (struct Frame *)malloc(30 * sizeof(struct Frame)); // 30 frames generated
    for (i = 0; i < 30; i++)
    {
        fr[i].type = DATA;
        fr[i].seq = (seqno + 1) % 2;
        // printf("check %d\n",fr->seq);
        fr[i].msg = "$Mbdv,gl;hpoihewadbjlszncx.mvc,bg;.nh’’wa?>SF<xdcfvgl;kjhgfdsasedrfg$";
        fr[i].len = strlen(fr->msg);
        seqno = fr[i].seq;
    }
    return fr;
}
int main()
{
    int sockfd, newsockfd, num;
    struct sockaddr_in serverAddress, clientAddress;
    struct Frame *frm;
    struct Frame f;
    int n, clientAddressLength, k;
    char msg[MAXSZ], msg1[MAXSZ];
    pid_t pid;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    // Socket creation
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // initialize the socket addresses
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(PORT);
    // bind
    bind(sockfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    // listen
    listen(sockfd, 5);
    int count = 0;
    while (count < 30)
    {
        printf("\n Waiting for Connection------\n");
        clientAddressLength = sizeof(clientAddress);
        // accept
        newsockfd = accept(sockfd, (struct sockaddr *)&clientAddress, &clientAddressLength);
        printf("Connected to client: %s\n", inet_ntoa(clientAddress.sin_addr));
        frm = makeframes();
        printf("\n How many frames to send?\n");
        scanf("%d", &num);
        int expAck;
        printf("Ready to send\n");
        // send
        while (count < num)
        {
            if (setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
                perror("setsockopt(SO_RCVTIMEO) failed");
            f = frm[count];
            sprintf(msg, "%d%d%s", f.seq, f.len, f.msg);
            if (f.len > 0)
            {
                k = send(newsockfd, msg, f.len, 0);

                if (k > 0)
                {
                    printf("Sent Frame %d\n", f.seq);
                    expAck = ((f.seq) + 1) % 2;
                    n = recv(newsockfd, msg1, MAXSZ, 0);
                    if (n == 0)
                        printf("no ack received\n");
                    if (n > 0)
                    {
                        printf("Received Ack %d\n", (msg1[0] - '0'));
                        if (expAck == (msg1[0] - '0'))
                            count++;
                    }
                }
            }
        }
        close(newsockfd);
        printf("Transfer successful\n");
        if (count == num)
            break;
    }
    return 0;
}