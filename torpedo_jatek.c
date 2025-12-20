#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h> 
#include <sys/select.h> // monitor FDs
#include <sys/types.h>  // for signal handling structures
#include <ctype.h>
#include <strings.h>    // for strcasecmp()
#include <signal.h>     // for signal handling
#include <stdbool.h>

// --- Configuration ---
#define SERIAL_PORT "/dev/ttyACM0" 
#define BAUD_RATE B115200 // Arduino uses 115200
#define MAX_LINE_LENGTH 128

// Running flag for graceful termination (modified by signal handler)
static volatile sig_atomic_t keep_running = 1;

// --- Serial Configuration Function ---
int serial_open_and_configure(const char *port_name, speed_t baud_rate) {
    int fd = open(port_name, O_RDWR | O_NOCTTY); // blocking open
    
    if (fd < 0) {
        perror("Error opening serial port");
        return -1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(tty));

    if (tcgetattr(fd, &tty) != 0) {
        perror("Error from tcgetattr");
        close(fd);
        return -1;
    }

    // Set Baud Rate
    cfsetospeed(&tty, baud_rate);
    cfsetispeed(&tty, baud_rate);

    // Standard raw configuration (8N1)
    tty.c_cflag &= ~(PARENB | CSTOPB | CSIZE); 
    tty.c_cflag |= (CS8 | CREAD | CLOCAL); 
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG); 
    tty.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); 
    tty.c_oflag &= ~(OPOST | ONLCR);  

    // Non-blocking reads (we use select() to wait)
    tty.c_cc[VTIME] = 0; 
    tty.c_cc[VMIN] = 0;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("Error from tcsetattr");
        close(fd);
        return -1;
    }

    return fd;
}

void print_manual(void) {
    printf("\n--- Torpedo Game Manual ---\n");
    printf("Local commands (typed in the terminal):\n");
    printf("  ? or help      - show this manual\n");
    printf("  exit or quit   - exit the program\n\n");
    printf("To send commands to the device, type them and press Enter\n");
    printf("Examples:\n");
    printf("  10 10 2 1 1     - send numbers/commands to the Arduino\n");
    printf("  restart         - restart the game\n");
    printf("  hit rate         - to print current game hit rate\n");
    printf("---------------------------\n\n");
    fflush(stdout);
}

void handle_sigint(int signo) {
    (void)signo;
    keep_running = 0; // safe operation in signal handler
}

// --- Main Program ---
int main() {
    int serial_fd = serial_open_and_configure(SERIAL_PORT, BAUD_RATE);
    
    if (serial_fd < 0) {
        fprintf(stderr, "Failed to initialize serial connection on %s\n", SERIAL_PORT);
        return 1;
    }

    // Register signal handlers for graceful shutdown using signal() to avoid sigaction struct
    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("signal(SIGINT)");
        close(serial_fd);
        return 1;
    }
    if (signal(SIGTERM, handle_sigint) == SIG_ERR) {
        // Non-fatal if we can't install SIGTERM handler, just warn
        perror("signal(SIGTERM)");
    }

    printf("--- TORPEDO GAME CLIENT ---\n");
    printf("Port opened at 115200 Baud. Enter commands (e.g., '10 10 2 1 1' or 'restart').\n");
    printf("Press CTRL+C to quit.\n");
    printf("---------------------------\n");

    // Show manual at startup
    print_manual();

    fd_set read_fds;
    int max_fd = (serial_fd > STDIN_FILENO ? serial_fd : STDIN_FILENO);
    int bytes_read;

    // Temporary buffer for keyboard input
    char input_buffer[MAX_LINE_LENGTH];

    bool send_restart = false;

    while (keep_running) {
        // 1. Reset the file descriptor set for select()
        FD_ZERO(&read_fds);
        FD_SET(serial_fd, &read_fds);       // Monitor the serial port
        FD_SET(STDIN_FILENO, &read_fds);    // Monitor the keyboard (stdin)

        // 2. Wait for activity on either descriptor (short timeout for responsiveness)
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0) {
            if (errno == EINTR) {
                if (!keep_running) break; // signal requested shutdown
                continue; // otherwise interrupted by unrelated signal
            }
            perror("select() error");
            break; 
        }

        // --- A. Handle Serial Data (READ from Arduino) ---
        if (FD_ISSET(serial_fd, &read_fds)) {
            char serial_data[256];
            bytes_read = read(serial_fd, serial_data, sizeof(serial_data) - 1);
            
            if (bytes_read > 0) {
                serial_data[bytes_read] = '\0';
                printf("%s", serial_data); // Print raw Arduino output
                fflush(stdout); 
            } else if (bytes_read < 0 && errno != EAGAIN) {
                perror("Serial read error");
                break;
            }
        }
        
        // --- B. Handle Keyboard Input (WRITE to Arduino or local commands) ---
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // Read line from keyboard (stdin)
            if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
                // Trim trailing whitespace/newline
                size_t len = strlen(input_buffer);
                while (len > 0 && isspace((unsigned char)input_buffer[len - 1])) {
                    input_buffer[--len] = '\0';
                }

                // Trim leading whitespace
                char *p = input_buffer;
                while (*p && isspace((unsigned char)*p)) p++;

                if (*p == '\0') {
                    continue; // empty input
                }

                // Local commands
                if (p[0] == '?' && p[1] == '\0') {
                    print_manual();
                    continue;
                } else if (strcasecmp(p, "help") == 0) {
                    print_manual();
                    continue;
                } else if (strcasecmp(p, "exit") == 0 || strcasecmp(p, "quit") == 0) {
                    // Request a restart on the Arduino before exiting
                    send_restart = true;
                    printf("Exiting by user request (will request device restart)...\n");
                    keep_running = 0;
                    break;
                } else if (strcasecmp(p, "restart") == 0) {
                    send_restart = true;
                }

                // Otherwise forward to serial device, ensure newline terminated
                char send_buf[MAX_LINE_LENGTH + 2];
                int send_len = snprintf(send_buf, sizeof(send_buf), "%s\n", p);
                ssize_t bytes_written = write(serial_fd, send_buf, send_len);
                if (bytes_written < 0) {
                    perror("Serial write error");
                    break;
                }

                printf(">> Sent: %s", send_buf);
                fflush(stdout); 
            }
        }
    }

    // If user requested restart on exit, send it before closing the port
    if (send_restart && serial_fd >= 0) {
        const char restart_msg[] = "restart\n";
        ssize_t w = write(serial_fd, restart_msg, sizeof(restart_msg) - 1);
        if (w < 0) {
            perror("Serial write error (restart)");
        } else {
            // Wait until bytes are transmitted
            if (tcdrain(serial_fd) != 0) {
                perror("tcdrain");
            } else {
                printf(">> Sent 'restart' to Arduino and drained output\n");
            }
        }
    }

    // Ensure the serial port is closed on exit
    if (serial_fd >= 0) {
        close(serial_fd);
        printf("\nSerial port closed. Exiting.\n");
    }

    return 0;
}