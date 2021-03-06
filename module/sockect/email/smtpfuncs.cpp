#include <stdio.h>
#include <string.h>
#include <time.h>
#include "smtpfuncs.h"
 
int send_mail(const char *smtpserver, const char *from, const char *to, 
                    const char *subject, const char *replyto, const char *msg)
{
    int n_socket;
    int n_retval = 0;
 
#ifdef WIN32
    startup_sockets_lib();
#endif
 
    /* First connect the socket to the SMTP server */
    if ((n_socket = connect_to_server(smtpserver)) == ERROR) 
        n_retval = E_NO_SOCKET_CONN;
 
    /* All connected. Now send the relevant commands to initiate a mail transfer */
    if (n_retval == 0 && send_command(n_socket, "MAIL From:<", from, ">\r\n", MAIL_OK) == ERROR)
        n_retval = E_PROTOCOL_ERROR;
    if (n_retval == 0 && send_command(n_socket, "RCPT To:<", to, ">\r\n", MAIL_OK) == ERROR) 
        n_retval = E_PROTOCOL_ERROR;
 
    /* Now send the actual message */
    if (n_retval == 0 && send_command(n_socket, "", "DATA", "\r\n", MAIL_GO_AHEAD) == ERROR) 
        n_retval = E_PROTOCOL_ERROR;
    if (n_retval == 0 && send_mail_message(n_socket, from, to, subject, replyto, msg) == ERROR) 
        n_retval = E_PROTOCOL_ERROR;
 
    /* Now tell the mail server that we're done */
    if (n_retval == 0 && send_command(n_socket, "", "QUIT", "\r\n", MAIL_GOODBYE) == ERROR) 
        n_retval = E_PROTOCOL_ERROR;
 
    /* Now close up the socket and clean up */
    if (close(n_socket) == ERROR) {
        fprintf(stderr, "Could not close socket.\n");
        n_retval = ERROR;
    }
 
#ifdef WIN32
    cleanup_sockets_lib();
#endif
 
    return n_retval;
}
 
int connect_to_server(const char *server)
{
    struct hostent *host;
    struct in_addr  inp;
    struct protoent *proto;
    struct sockaddr_in sa;
    int n_sock;
#define SMTP_PORT      25
#define BUFSIZE     4096
    char s_buf[BUFSIZE] = "";
    int n_ret;
 
    /* First resolve the hostname */
    host = gethostbyname(server);
    if (host == NULL) {
        fprintf(stderr, "Could not resolve hostname %s. Aborting...\n", server);
        return ERROR;
    }
 
    memcpy(&inp, host->h_addr_list[0], host->h_length);
 
    /* Next get the entry for TCP protocol */
    if ((proto = getprotobyname("tcp")) == NULL) {
        fprintf(stderr, "Could not get the protocol for TCP. Aborting...\n");
        return ERROR;   
    }
 
    /* Now create the socket structure */
    //if ((n_sock = socket(PF_INET, SOCK_STREAM, proto->p_proto)) == INVALID_SOCKET) {
    if ((n_sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "Could not create a TCP socket. Aborting...\n");
        return ERROR;
    } /* Now connect the socket */
    memset(&sa, 0, sizeof(sa));
    sa.sin_addr = inp;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(25);

    fprintf(stderr, "start to connection by host %s. %s\n", server, inet_ntoa(sa.sin_addr));
    if (connect(n_sock, (struct sockaddr *)&sa, sizeof(sa)) == SOCKET_ERROR) {
        fprintf(stderr, "Connection refused by host %s.", server);
        return ERROR;
    }
 
    /* Now read the welcome message */
    n_ret = recv(n_sock, s_buf, BUFSIZE, 0);
    fprintf(stderr, "%d welcome by host %s .", n_ret, server);
     
    return n_sock;
}
 
int send_command(int n_sock, const char *prefix, const char *cmd, 
                        const char *suffix, int ret_code)
{
#define BUFSIZE     4096
    char s_buf[BUFSIZE] = "";
    char s_buf2[50];
     
    strncpy(s_buf, prefix, BUFSIZE);
    strncat(s_buf, cmd, BUFSIZE);
    strncat(s_buf, suffix, BUFSIZE);
 
    if (send(n_sock, s_buf, strlen(s_buf), 0) == SOCKET_ERROR) {
        fprintf(stderr, "Could not send command string %s to server.", s_buf);
        return ERROR;
    }
 
    /* Now read the response. */
    recv(n_sock, s_buf, BUFSIZE, 0);
 
    /* Now check if the ret_code is in the buf */
    sprintf(s_buf2, "%d", ret_code);
 
    if (strstr(s_buf, s_buf2) != NULL)
        return TRUE;
    else
        return ERROR;
}
 
int send_mail_message(int n_sock, const char *from, const char *to,
                            const char *subject, const char *replyto, const char *msg)
{
#define BUFSIZE     4096
#define BUFSIZE2    100
#define MSG_TERM    "\r\n.\r\n"
#define MAIL_AGENT  "Mayukh's SMTP code (http://www.mayukhbose.com/freebies/c-code.php)"
    char s_buf[BUFSIZE];
    char s_buf2[BUFSIZE2];
    time_t t_now = time(NULL);
    int n_ret;
 
    /* First prepare the envelope */
    strftime(s_buf2, BUFSIZE2, "%a, %d %b %Y  %H:%M:%S +0000", gmtime(&t_now));
 
    snprintf(s_buf, BUFSIZE, "Date: %s\r\nFrom: %s\r\nTo: %s\r\nSubject: %s\r\nX-Mailer: %s\r\nReply-To: %s\r\n\r\n",
                s_buf2, from, to, subject, MAIL_AGENT, replyto); 
 
    /* Send the envelope */
    if (send(n_sock, s_buf, strlen(s_buf), 0) == SOCKET_ERROR) {
        fprintf(stderr, "Could not send message header: %s", s_buf);
        return ERROR;
    }
 
    /* Now send the message */
    if (send(n_sock, msg, strlen(msg), 0) == SOCKET_ERROR) {
        fprintf(stderr, "Could not send the message %s\n", msg);
        return ERROR;
    }
 
    /* Now send the terminator*/
    if (send(n_sock, MSG_TERM, strlen(MSG_TERM), 0) == SOCKET_ERROR) {
        fprintf(stderr, "Could not send the message terminator.\n");
        return ERROR;
    }
 
    /* Read and discard the returned message ID */
    n_ret = recv(n_sock, s_buf, BUFSIZE, 0);
 
    return TRUE;
}
 
#ifdef WIN32
/* 
 * Routines for  Windows Socket Library
 */
int startup_sockets_lib(void)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int n_err;
  
    wVersionRequested = MAKEWORD( 1, 1 );
  
    n_err = WSAStartup( wVersionRequested, &wsaData );
    if (n_err != 0) {
        fprintf(stderr, "Could not find winsock.dll.Aborting...\n");
        return FALSE;
    }
   
    if (LOBYTE( wsaData.wVersion ) != 1 ||
            HIBYTE( wsaData.wVersion ) != 1) {
        fprintf(stderr, "Could not find w1.1 version of winsock.dll.Aborting...\n");
        WSACleanup();
        return FALSE; 
    }
  
    return TRUE; 
}
 
int cleanup_sockets_lib(void)
{
    WSACleanup();
    return TRUE;
}
 
#endif
