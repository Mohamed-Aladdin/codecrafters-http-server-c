#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage

	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
	 	return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									.sin_port = htons(4221),
									.sin_addr = { htonl(INADDR_ANY) },
									};

	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	const int client_fd =
		accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if (client_fd < 0) {
		printf("Accept failed: %s \n", strerror(errno));
		return 1;
	}
	printf("Client connected\n");

	char req_buffer[BUFFER_SIZE];
	char res[BUFFER_SIZE];

	if (read(client_fd, req_buffer, BUFFER_SIZE) < 0) {
		printf("Read failed: %s \n", strerror(errno));
		return 1;
	} else {
		printf("Request from client: %s\n", req_buffer);
	}

	char *path = strtok(req_buffer, " ");
	path = strtok(NULL, " ");

	char *res_ok = "HTTP/1.1 200 OK\r\n\r\n";
	char *res_not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
	char *format = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
                        "%ld\r\n\r\n%s";

	if (strncmp(path, "/user-agent", 11) == 0) {
		strtok(0, "\r\n");
		strtok(0, "\r\n");
		char *userAgent = strtok(0, "\r\n") + 12;
		sprintf(res, format, strlen(userAgent), userAgent);
	} else if (strncmp(path, "/echo/", 6) == 0) {
		char *content = path + 6;
		sprintf(res, format, strlen(content), content);
	} else {
		snprintf(res, sizeof(res), "%s", (strcmp(path, "/") == 0) ? res_ok : res_not_found);
	}

	int bytes_sent = send(client_fd, res, strlen(res), 0);

	if (bytes_sent < 0) {
		printf("Error: %s \n", strerror(errno));
	}

	close(server_fd);
	close(client_fd);

	return 0;
}
