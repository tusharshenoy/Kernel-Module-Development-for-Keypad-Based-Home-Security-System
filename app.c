#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/securitysystem"
#define PASSWORD_MAX_LENGTH 5

// Custom IOCTL commands
#define IOCTL_SET_PASSWORD _IOW('a', 1, char *)
#define IOCTL_RESET_PASSWORD _IO('a', 2)

void display_menu() {
    printf("\nSecurity System Menu:\n");
    printf("1. Set Password\n");
    printf("2. Reset Password\n");
    printf("3. Exit\n");
}

int main() {
    int choice, fd;
    char password[PASSWORD_MAX_LENGTH];

    // Open the device file
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    while (1) {
        display_menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter new password (max 4 digits) between 0 to 9 and A B C D: ");
                scanf("%4s", password);  // Read up to 4 characters
                password[PASSWORD_MAX_LENGTH - 1] = '\0';  //  Ensure null-termination

                // Use ioctl to set the password
                if (ioctl(fd, IOCTL_SET_PASSWORD, password) < 0) {
                    perror("Failed to set password");
                } else {
                    printf("Password set successfully.\n");
                }
                break;

            case 2:
                // Use ioctl to reset the password
                if (ioctl(fd, IOCTL_RESET_PASSWORD) < 0) {
                    perror("Failed to reset password");
                } else {
                    printf("Password reset successfully.\n");
                }
                break;

            case 3:
                close(fd);  // Close the device file
                printf("Exiting...\n");
                return EXIT_SUCCESS;

            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
}
