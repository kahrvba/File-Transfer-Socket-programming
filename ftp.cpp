#include <iostream> // Include the iostream standard library for input/output operations.
#include <fstream> // Include the fstream standard library for file stream operations.
#include <sstream> // Include the sstream standard library for string stream operations.
#include <string> // Include the string standard library for string operations.
#include <vector> // Include the vector standard library for dynamic array operations.
#include <thread> // Include the thread standard library for multithreading operations.
#include <mutex> // Include the mutex standard library for mutual exclusion operations.
#include <netinet/in.h> // Include the netinet/in.h header for network operations.
#include <sys/socket.h> // Include the sys/socket.h header for socket operations.
#include <unistd.h> // Include the unistd.h header for POSIX operating system API.
#include <filesystem> // Include the filesystem standard library for filesystem operations.
#include <map> // Include the map standard library for associative array operations.
#include <cstring> // Include the cstring standard library for string operations.
#include <arpa/inet.h> // Include the arpa/inet.h header for internet operations.

std::mutex mtx; // Declare a global mutex for synchronizing access to shared resources.

// Server functions
void handle_client(int client_socket, const std::string& directory, const std::map<std::string, std::string>& users);
void start_server(const std::string& directory, int port, const std::map<std::string, std::string>& users);
std::map<std::string, std::string> parse_users(const std::string& filename);

// Client function
void start_client(const std::string& server_ip, int port);

int main(int argc, char* argv[]) {
    // Check if the number of command-line arguments is less than 2.
    if (argc < 2) {
        // Output an error message to the standard error stream.
        std::cerr << "Usage: " << argv[0] << " [server|client] [options]\n";
        // Return EXIT_FAILURE to indicate an error occurred.
        return EXIT_FAILURE;
    }

    // Extract the mode from the command-line arguments.
    std::string mode = argv[1];

    // Check if the mode is "server".
    if (mode == "server") {
        // Initialize the directory to the current working directory.
        std::string directory = ".";
        // Initialize the port to 8080.
        int port = 8080;
        // Initialize the password file to "passwords.txt".
        std::string password_file = "passwords.cfg";

        // Declare an integer variable to store the option character.
        int opt;
        // Loop through the command-line options.
        while ((opt = getopt(argc - 1, argv + 1, "d:p:u:")) != -1) {
            // Switch on the option character.
            switch (opt) {
                // If the option is 'd', set the directory to the option argument.
                case 'd': directory = optarg; break;
                // If the option is 'p', set the port to the integer value of the option argument.
                case 'p': port = std::stoi(optarg); break;
                // If the option is 'u', set the password file to the option argument.
                case 'u': password_file = optarg; break;
                // If the option is not recognized, output an error message and return EXIT_FAILURE.
                default: std::cerr << "Usage: " << argv[0] << " server -d directory -p port -u password_file\n"; return EXIT_FAILURE;
            }
        }

        // Parse the users from the password file.
        auto users = parse_users(password_file);
        // Start the server with the specified directory, port, and users.
        start_server(directory, port, users);
    } else if (mode == "client") {
        // Check if the number of command-line arguments is less than 4.
        if (argc < 4) {
            // Output an error message to the standard error stream.
            std::cerr << "Usage: " << argv[0] << " client <server_ip> <port>\n";
            // Return EXIT_FAILURE to indicate an error occurred.
            return EXIT_FAILURE;
        }

        // Extract the server IP from the command-line arguments.
        std::string server_ip = argv[2];
        // Extract the port from the command-line arguments and convert it to an integer.
        int port = std::stoi(argv[3]);

        // Start the client with the specified server IP and port.
        start_client(server_ip, port);
    } else {
        // Output an error message to the standard error stream for an unknown mode.
        std::cerr << "Unknown mode: " << mode << "\n";
        // Return EXIT_FAILURE to indicate an error occurred.
        return EXIT_FAILURE;
    }

    // Return EXIT_SUCCESS to indicate that the program has run successfully.
    return 0;
}
void handle_client(int client_socket, const std::string& directory, const std::map<std::string, std::string>& users) {
    char buffer[1024]; // buffer to store incoming data from the client
    bool authenticated = false; // flag to track if the client is authenticated
    std::string username; // variable to store the authenticated username

    while (true) {
        memset(buffer, 0, sizeof(buffer)); // clear the buffer before receiving new data
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0); // receive data from the client.
        if (bytes_received <= 0) break; // if no data received, exit the loop

        std::istringstream iss(buffer); // create a string stream to parse the received data
        std::string command; // variable to store the command extracted from the received data
        iss >> command; // extract the command from the received data

        if (command == "USER") { // check if the command is for user authentication
            std::string user, pass; // Variables to store the username and password
            iss >> user >> pass; // extract the username and password from the received data
            // Validate user credentials
            if (users.find(user) != users.end() && users.at(user) == pass) {
                authenticated = true; // set authentication flag to true
                username = user;  // store the authenticated username
                send(client_socket, "200 User authenticated.\n", 24, 0); // send authentication success message to the client
            } else {
                send(client_socket, "400 User not found or password incorrect.\n", 43, 0); // send authentication failure message
            }
        } else if (authenticated) { // if client is authenticated, process commands
            if (command == "LIST") { // handle the LIST command to list files in the directory
                std::lock_guard<std::mutex> lock(mtx); // lock to ensure thread safety
                std::stringstream list; // string stream to build the list of files
                for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                    list << entry.path().filename().string() << " " << std::filesystem::file_size(entry.path()) << "\n"; // append file information to the list
                }
                list << ".\n"; // add a delimiter to mark the end of the list
                send(client_socket, list.str().c_str(), list.str().size(), 0);// send the file list to the client
            } else if (command == "GET") {
                std::string filename; // Variable to store the filename
                iss >> filename; // Extract the filename from the received data
                std::lock_guard<std::mutex> lock(mtx);// Lock to ensure thread safety
                std::ifstream file(directory + "/" + filename, std::ios::binary); // Open the file for reading in binary mode
                if (file.is_open()) { // Check if the file is successfully opened
                    std::stringstream file_content; // String stream to store file content
                    file_content << file.rdbuf(); // Read the file content into the string stream
                    file_content << "\r\n.\r\n";// Add a delimiter to mark the end of the file content
                    send(client_socket, file_content.str().c_str(), file_content.str().size(), 0);// Send the file content to the client
                    file.close();; // Close the file after sending
                } else {
                    send(client_socket, "404 File not found.\n", 20, 0); // Send a message indicating file not found
                }
	    } else if (command == "PUT") {// Check if the command is to store a file on the server
                std::string filename;// Variable to store the filename
                iss >> filename;// extract the filename from the received data
                std::lock_guard<std::mutex> lock(mtx);// lock to ensure thread safety
                std::ofstream file(directory + "/" + filename, std::ios::binary);// open the file for writing in binary mode
                if (file.is_open()) {// check if the file is successfully opened
                    std::string file_content((std::istreambuf_iterator<char>(iss)), std::istreambuf_iterator<char>());// read the file content from the stream
                    file.write(file_content.c_str(), file_content.size());// write the file content to the file
                    file.close(); // close the file after writing
                    std::string response = "200 " + std::to_string(file_content.size()) + " Bytes transferred.\n";// create a success response message
                    send(client_socket, response.c_str(), response.size(), 0); // send the success response to the client
                } else {
                    send(client_socket, "400 File cannot be saved on server.\n", 37, 0); // send a message indicating file saving failure
                }
