#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <zlib.h>

#define BUFFER_SIZE 1024

char *dir;

void *request_handler(void *cfd) {
	int client_fd = *(int*)cfd;
	char req_buffer[BUFFER_SIZE];
	char res[BUFFER_SIZE];

	// If read is failed we print the error and exit
	if (read(client_fd, req_buffer, BUFFER_SIZE) < 0) {
		printf("Read failed: %s \n", strerror(errno));
		return NULL;
	} else {
		printf("Request from client: %s\n", req_buffer);
	}

	char *body = strstr(req_buffer, "\r\n\r\n") + 4;
	char *accept_encoding = strstr(req_buffer, "Accept-Encoding: ");
	char *method = strtok(req_buffer, " ");
	char *path = strtok(NULL, " ");
	char *res_ok = "HTTP/1.1 200 OK\r\n\r\n";
	char *res_created = "HTTP/1.1 201 Created\r\n\r\n";
	char *res_not_found = "HTTP/1.1 404 Not Found\r\n\r\n";
	char *res_bad_request = "HTTP/1.1 400 Bad Request\r\n\r\n";
	char *res_srvr_err = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
	char *format =
		"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
		"%ld\r\n\r\n%s";

	// Flag to check and store whether encoding's been provided or not
	int gzip_accepted = 0;
	if (accept_encoding) {
		char *encoding_value = strtok(accept_encoding + strlen("Accept-Encoding: "), "\r\n");
		if (encoding_value && strstr(encoding_value, "gzip")) {
			gzip_accepted = 1;
		}
	}

	// Logic behind different request formats
	if (strncmp(path, "/files/", 7) == 0 && strncmp(method, "POST", 4) == 0) {
		char *file = path + 7;
		char f_path[BUFFER_SIZE];
		snprintf(f_path, sizeof(f_path), "%s/%s", dir, file);

		// Read headers to get Content-Length
		printf("Body: %s, ReqBuff: %s", body, req_buffer);
		if (!body) {
			snprintf(res, sizeof(res), "%s", res_bad_request);
			send(client_fd, res, strlen(res), 0);
			close(client_fd);
			return NULL;
		}

		FILE *file_fd = fopen(f_path, "w");

		if (file_fd) {
			fwrite(body, 1, strlen(body), file_fd);
			fclose(file_fd);
			snprintf(res, sizeof(res), "%s", res_created);
		} else {
			snprintf(res, sizeof(res), "%s", res_srvr_err);
		}
	} else if (strncmp(path, "/files/", 7) == 0 && strncmp(method, "GET", 3) == 0) {
		char *file = strchr(path + 1, '/');
		printf("Filename: %s\n", file);

		if (file) {
			char f_path[BUFFER_SIZE];
			snprintf(f_path, sizeof(f_path), "%s%s", dir, file);
			FILE *file_fd = fopen(f_path, "r");

			if (file_fd) {
				char file_buff[BUFFER_SIZE];
				int br = fread(file_buff, 1, BUFFER_SIZE - 1, file_fd);
				file_buff[br] = '\0';
				fclose(file_fd);

				int res_size = snprintf(res, sizeof(res),
            				"HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: "
            				"%d\r\n\r\n%s",
            				br, file_buff);
				if (res_size >= sizeof(res)) {
					fprintf(stderr, "Response buffer overflow detected\n");
					close(client_fd);
					return NULL;
				}
			} else {
				snprintf(res, sizeof(res), "%s", res_not_found);
			}
		} else {
			snprintf(res, sizeof(res), "%s", res_not_found);
		}
	} else if (strncmp(path, "/user-agent", 11) == 0) {
		strtok(0, "\r\n");
		strtok(0, "\r\n");
		char *userAgent = strtok(0, "\r\n") + 12;
		sprintf(res, format, strlen(userAgent), userAgent);
	} else if (strncmp(path, "/echo/", 6) == 0) {
		char *content = path + 6;
		if (gzip_accepted) {
			snprintf(res, sizeof(res),
				"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Encoding: gzip\r\nContent-Length: %ld\r\n\r\n%s", strlen(content), content);
		} else {
			sprintf(res, format, strlen(content), content);
		}
	} else {
		snprintf(res, sizeof(res), "%s",
			(strcmp(path, "/") == 0) ? res_ok : res_not_found);
	}

	int bytes_sent = send(client_fd, res, strlen(res), 0);

	if (bytes_sent < 0) {
		printf("Error: %s \n", strerror(errno));
	}

	close(client_fd);
}

int main(int argc, char **argv) {
	const char *welcome =
	"                                                                                                  \n"
	"                                                                                                  \n"
	"   SSSSSSSSSSSSSSS                                                                                \n"
	" SS:::::::::::::::S                                                                               \n"
	"S:::::SSSSSS::::::S                                                                               \n"
	"S:::::S     SSSSSSS                                                                               \n"
	"S:::::S                eeeeeeeeeeee    rrrrr   rrrrrrrrrvvvvvvv           vvvvvvv eeeeeeeeeeee    \n"
	"S:::::S              ee::::::::::::ee  r::::rrr:::::::::rv:::::v         v:::::vee::::::::::::ee  \n"
	" S::::SSSS          e::::::eeeee:::::eer:::::::::::::::::rv:::::v       v:::::ve::::::eeeee:::::ee\n"
	"  SS::::::SSSSS    e::::::e     e:::::err::::::rrrrr::::::rv:::::v     v:::::ve::::::e     e:::::e\n"
	"    SSS::::::::SS  e:::::::eeeee::::::e r:::::r     r:::::r v:::::v   v:::::v e:::::::eeeee::::::e\n"
	"       SSSSSS::::S e:::::::::::::::::e  r:::::r     rrrrrrr  v:::::v v:::::v  e:::::::::::::::::e \n"
	"            S:::::Se::::::eeeeeeeeeee   r:::::r               v:::::v:::::v   e::::::eeeeeeeeeee  \n"
	"            S:::::Se:::::::e            r:::::r                v:::::::::v    e:::::::e           \n"
	"SSSSSSS     S:::::Se::::::::e           r:::::r                 v:::::::v     e::::::::e          \n"
	"S::::::SSSSSS:::::S e::::::::eeeeeeee   r:::::r                  v:::::v       e::::::::eeeeeeee  \n"
	"S:::::::::::::::SS   ee:::::::::::::e   r:::::r                   v:::v         ee:::::::::::::e  \n"
	" SSSSSSSSSSSSSSS       eeeeeeeeeeeeee   rrrrrrr                    vvv            eeeeeeeeeeeeee  \n"
	"                                                                                                  \n"
	"                                                                                                  \n"
	"                                                                                                  \n"
	"                                                                                                  \n"
	"                                                                                                  \n"
	"                                                                                                  \n";


	// Print the ASCII art
	printf("%s", welcome);

	if (argc >= 2 && (strncmp(argv[1], "--directory", 11) == 0)) {
		dir = argv[2];
	}
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	pthread_t tid;

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

	// While loop allows the pthread library to function and handle concurrent sessions
	while (1) {
			client_addr_len = sizeof(client_addr);
			int *client_fd = malloc(sizeof(int));
			if (!client_fd) {
				printf("Memory allocation failed\n");
				close(server_fd);
				return 1;
			}
			*client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);

			if (client_fd < 0) {
				printf("Accept failed: %s \n", strerror(errno));
				free(client_fd);
				close(server_fd);
				return 1;
			}
			pthread_create(&tid, NULL, request_handler, client_fd);
			pthread_detach(tid);
			printf("Client connected\n");
	}

	 close(server_fd);
	 return 0;
}
