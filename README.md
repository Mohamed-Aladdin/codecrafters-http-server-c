[![progress-banner](https://backend.codecrafters.io/progress/http-server/8e39bac6-a5f3-477c-b318-ff8002e37bef)](https://app.codecrafters.io/users/codecrafters-bot?r=2qF)

# Serve (HTTP Server with headers compression)

## Introduction
This project is a custom HTTP server implemented in C that supports gzip compression. The server is designed to handle HTTP requests, respond with appropriate content, and compress responses using gzip when requested by the client. The main goal of this project is to demonstrate the implementation of HTTP protocols, multi-threading, and data compression techniques.

- **Deployed Site:** [https://mohamed-aladdin.github.io/codecrafters-http-server-c/](#)
- **Project Blog Article:** [Final Project Blog Article](#) <!-- Replace with actual link -->
- **Author(s) LinkedIn:**
  - [Mohamed Aladdin](https://www.linkedin.com/in/m-aladdin101)

## Installation
To set up the project locally, follow these steps:

1. **Clone the repository:**
   ```sh
   git clone https://github.com/Mohamed-Aladdin/codecrafters-http-server-c.git
   cd codecrafters-http-server-c
2. **Install required dependencies:**
   Ensure you have the zlib library installed. On a Debian-based system, you can install it using:
   ```sh
   sudo apt-get install zlib1g-dev
3. **Compile the project:**
   ```sh
   gcc -o server app/server.c -pthread -lz

Usage
To run the server, use the following command:
```sh
./server --directory <directory_path>
Replace <directory_path> with the path to the directory where your files are stored.
```sh
./server --directory /var/www/html

Making Requests:
To test the server, you can use curl or any other HTTP client. For example:
```sh
curl -v -H "Accept-Encoding: gzip" http://localhost:4221/echo/abc

Contributing
Contributions are welcome! Please follow these steps to contribute:

1. Fork the repository.
2. Create a new branch (git checkout -b feature/your-feature).
3. Commit your changes (git commit -am 'Add some feature').
4. Push to the branch (git push origin feature/your-feature).
5. Open a pull request.

Related Projects
Here are some related projects that you might find interesting:
**Codecrafter's HTTP Server:** [https://app.codecrafters.io/courses/http-server](#)

Licensing
This project is licensed under the MIT License. See the LICENSE file for more details.

This `README.md` provides a comprehensive guide for users and contributors.
