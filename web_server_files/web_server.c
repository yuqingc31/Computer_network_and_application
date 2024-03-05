#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#include "config.h"
#include "helpers.h"

/*------------------------------------------------------------------------
 * Program:   http server
 *
 * Purpose:   allocate a socket and then repeatedly execute the following:
 *              (1) wait for the next connection from a client
 *              (2) read http request, reply to http request
 *              (3) close the connection
 *              (4) go back to step (1)
 *
 * Syntax:    http_server [ port ]
 *
 *               port  - protocol port number to use
 *
 * Note:      The port argument is optional.  If no port is specified,
 *            the server uses the port specified in config.h
 *
 *------------------------------------------------------------------------
 */

int main(int argc, char *argv[])
{
  /* structure to hold server's and client addresses, respectively */
  struct sockaddr_in server_address, client_address;

  int listen_socket = -1;    // 用于侦听和接受新请求的套接字
  int connection_socket = -1;  // 保存与客户端的连接的套接字
  int port = 0;   // 用于保存服务器监听的端口号

  /* id of child process to handle request */
  pid_t pid = 0;   // 用于保存子进程的ID，用于处理客户端的请求

  char response_buffer[MAX_HTTP_RESPONSE_SIZE] = "";   // 用于保存服务器将要发送回客户端的响应内容
  // 用于保存将要发送给客户端的HTTP响应的状态码和短语
  int status_code = -1;
  char *status_phrase = "";

  /* 1) Create a socket */
  /* START CODE SNIPPET 1 */
  listen_socket = socket(AF_INET, SOCK_STREAM, 0);
  /* END CODE SNIPPET 1 */

  /* Check command-line argument for port and extract
   * port number if one is specified. Otherwise, use default
   */
  if (argc > 1)
  {
    /* Convert from string to integer */
    port = atoi(argv[1]);
  }
  else
  {
    port = DEFAULT_PORT;
  }

  if (port <= 0)
  {
    /* Test for legal value */
    fprintf(stderr, "bad port number %d\n", port);
    exit(EXIT_FAILURE);
  }

  /* Clear the server address */
  memset(&server_address, 0, sizeof(server_address));

  /* 2) Set the values for the server address structure */
  /* START CODE SNIPPET 2 */
  server_address. sin_family = AF_INET;   // 设置地址族
  server_address. sin_port = htons(port);   // 设置端口号
  server_address. sin_addr. s_addr = INADDR_ANY;   // INADDR_ANY表示服务器将接受任何IP地址的连接
  /* END CODE SNIPPET 2 */

  /* 3) Bind the socket to the address information set in server_address */
  /* START CODE SNIPPET 3 */
  bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address));
  /* END CODE SNIPPET 3 */

  /* 4) Start listening for connections */
  /* START CODE SNIPPET 4 */

  // 接受方法将阻塞，直到建立连接。
  listen(listen_socket, QLEN);

  /* END CODE SNIPPET 4 */

  /* Main server loop
   * Loop while the listen_socket is valid
   */
  while (listen_socket >= 0)
  {
    /* 5) Accept a connection */
    /* START CODE SNIPPET 5 */
    connection_socket = accept (listen_socket, NULL, NULL ) ;
    // socklen_t client_address_length = sizeof(client_address);
    // connection_socket = accept(listen_socket, (struct sockaddr *) &client_address, &client_address_length);
    // if (connection_socket < 0) {
    //     perror("Error accepting connection");
    //     exit(EXIT_FAILURE);
    // }
    /* END CODE SNIPPET 5 */

    /* Fork a child process to handle this request */
    if ((pid = fork()) == 0)
    {
      /*----------START OF CHILD CODE----------------*/
      /* We are now in the child process */

      /* Close the listening socket
       * The child process does not need access to listen_socket 
       */
      if (close(listen_socket) < 0)
      {
        fprintf(stderr, "child couldn't close listen socket\n");
        exit(EXIT_FAILURE);
      }

      /* See httpreq.h for definition */
      struct http_request new_request;
      /* 6) call helper function to read the request
       * this will fill in the struct new_request for you
       * see helper.h and httpreq.h                      
       */
      /* START CODE SNIPPET 6 */
      Parse_HTTP_Request(connection_socket, &new_request);
      /* END CODE SNIPPET 6 */

      /* 7) Decide which status_code and reason phrase to return to client */
      /* START CODE SNIPPET 7 */
      
      // 如果找到请求的资源
      if (Is_Valid_Resource(new_request.URI)) {
        status_code = 200;
        status_phrase = "OK";
      } 
      // 如果找不到请求的资源
      else if(!Is_Valid_Resource(new_request.URI)){
        status_code = 404;
        status_phrase = "Not Found";
      }
      // 如果请求是HEAD 
      else if (strcmp(new_request.method, "HEAD") == 0) {
        status_code = 200;
        status_phrase = "OK";
      } 
      // 如果请求的方法未实现
      else if (!strcmp(new_request.method, "HEAD") == 0){
        status_code = 501;
        status_phrase = "Not Implemented";
      } 
      // 如果请求的资源无效
      else {
        status_code = 502;
        status_phrase = "Bad Gateway";
      }

      
      /* END CODE SNIPPET 7 */

      /* 8) Set the reply message to the client
       * Copy the following line and fill in the ??
       * sprintf(response_buffer, "HTTP/1.0 %d %s\r\n", ??, ??);
       */
      /* START CODE SNIPPET 8 */
      sprintf(response_buffer, "HTTP/1.0 %d %s\r\n", status_code, status_phrase);
      /* END CODE SNIPPET 8 */

      printf("Sending response line: %s\n", response_buffer);

      /* 9) Send the reply message to the client
       * Copy the following line and fill in the ??
       * send(??, response_buffer, strlen(response_buffer), 0);
       */
      /* START CODE SNIPPET 9 */
      send(connection_socket, response_buffer, strlen(response_buffer), 0);
      /* END CODE SNIPPET 9 */

      bool is_ok_to_send_resource = false;
      /* 10) Send resource (if requested) under what condition will the
       * server send an entity body?
       */
      /* START CODE SNIPPET 10 */
      if (strcmp(new_request.method, "HEAD") == 0) {
        is_ok_to_send_resource = false;
      } else if (status_code == 200) {
        is_ok_to_send_resource = true;
      }
      /* END CODE SNIPPET 10 */

      if (is_ok_to_send_resource)
      {
        Send_Resource(connection_socket, new_request.URI);
      }
      else
      {
        /* 11) Do not send resource
         * End the HTTP headers
         * Copy the following line and fill in the ??
         * send(??, "\r\n\r\n", strlen("\r\n\r\n"), 0);
         */
        /* START CODE SNIPPET 11 */
        send(connection_socket, "\r\n\r\n", strlen("\r\n\r\n"), 0);
        /* END CODE SNIPPET 11 */
      }

      /* Child's work is done
       * Close remaining descriptors and exit 
       */
      if (connection_socket >= 0)
      {
        if (close(connection_socket) < 0)
        {
          fprintf(stderr, "closing connected socket failed\n");
          exit(EXIT_FAILURE);
        }
      }

      /* All done return to parent */
      exit(EXIT_SUCCESS);
    }
    /*----------END OF CHILD CODE----------------*/

    /* Back in parent process
     * Close parent's reference to connection socket,
     * then back to top of loop waiting for next request 
     */
    if (connection_socket >= 0)
    {
      if (close(connection_socket) < 0)
      {
        fprintf(stderr, "closing connected socket failed\n");
        exit(EXIT_FAILURE);
      }
    }

    /* if child exited, wait for resources to be released */
    waitpid(-1, NULL, WNOHANG);
  }
}
