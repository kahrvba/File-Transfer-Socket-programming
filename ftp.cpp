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
        std::string password_file = "passwords.txt";

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
