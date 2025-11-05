#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#define DEVICE_PATH "/dev/xiaomi-touch"
#define VERSION "1.0"

#define Touch_Game_Mode 0
#define Touch_Active_MODE 1
#define Touch_UP_THRESHOLD 2
#define Touch_Tolerance 3
#define Touch_Aim_Sensitivity 4
#define Touch_Tap_Stability 5
#define Touch_Expert_Mode 6
#define Touch_Edge_Filter 7
#define Touch_Panel_Orientation 8
#define Touch_Report_Rate 9
#define Touch_Fod_Enable 10
#define Touch_Aod_Enable 11
#define Touch_Resist_RF 12
#define Touch_Idle_Time 13
#define Touch_Doubletap_Mode 14
#define Touch_Grip_Mode 15
#define Touch_FodIcon_Enable 16
#define Touch_Nonui_Mode 17
#define Touch_Debug_Level 18
#define Touch_Power_Status 19
#define Touch_Pen_ENABLE 20
#define Touch_Mode_NUM 21

typedef struct {
    const char *name;
    const char *description;
    int modes[5][2];
    int count;
} TouchProfile;

TouchProfile profiles[] = {
    {"gaming", "Best for games", {{0, 1}, {2, 5}, {4, 5}, {9, 1}, {1, 1}}, 5},
    {"sensitive", "High sensitivity", {{2, 5}, {3, 5}, {4, 5}, {5, 5}, {-1, -1}}, 4},
    {"normal", "Default settings", {{0, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}}, 5},
    {"battery", "Save battery", {{9, 0}, {1, 0}, {13, 1}, {-1, -1}, {-1, -1}}, 3}
};

const char *mode_names[] = {
    "Game Mode", "Active Mode", "UP Threshold", "Tolerance",
    "Aim Sensitivity", "Tap Stability", "Expert Mode", "Edge Filter",
    "Panel Orientation", "Report Rate", "FOD Enable", "AOD Enable",
    "Resist RF", "Idle Time", "Double Tap", "Grip Mode",
    "FOD Icon", "Non-UI Mode", "Debug Level", "Power Status", "Pen Enable"
};

void show_help();
void show_profiles();
void show_modes();
int set_mode(int fd, int mode, int value);
int apply_profile(int fd, const char *name);

int main(int argc, char *argv[]) {
    // shelp no arguments
    if (argc == 1) {
        show_help();
        return EXIT_SUCCESS;
    }

    // help
    if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0) {
        show_help();
        return EXIT_SUCCESS;
    }

    // list profiles
    if (strcmp(argv[1], "list") == 0) {
        show_profiles();
        return EXIT_SUCCESS;
    }

    // list mode
    if (strcmp(argv[1], "modes") == 0) {
        show_modes();
        return EXIT_SUCCESS;
    }

    // versiiiiooon
    if (strcmp(argv[1], "version") == 0) {
        printf("Xiaomi Touch Control v%s\n", VERSION);
        return EXIT_SUCCESS;
    }

    // open device
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        printf("Error: Cannot open device. Need root access.\n");
        printf("Try: su -c 'xiaomi-touch %s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    int ret = EXIT_SUCCESS;

    // profile command: xiaomi-touch gaming
    if (argc == 2) {
        ret = apply_profile(fd, argv[1]);
    }
    // set command: xiaomi-touch set 4 5
    else if (strcmp(argv[1], "set") == 0 && argc == 4) {
        int mode = atoi(argv[2]);
        int value = atoi(argv[3]);
        ret = set_mode(fd, mode, value);
    }
    // on/off commands: xiaomi-touch on 0 or xiaomi-touch off 0
    else if (strcmp(argv[1], "on") == 0 && argc == 3) {
        int mode = atoi(argv[2]);
        int value = (mode >= 2 && mode <= 5) ? 5 : 1;
        ret = set_mode(fd, mode, value);
    }
    else if (strcmp(argv[1], "off") == 0 && argc == 3) {
        int mode = atoi(argv[2]);
        ret = set_mode(fd, mode, 0);
    }
    else {
        printf("Error: Invalid command\n\n");
        show_help();
        ret = EXIT_FAILURE;
    }

    close(fd);
    return ret;
}

int set_mode(int fd, int mode, int value) {
    if (mode < 0 || mode >= Touch_Mode_NUM) {
        printf("Error: Mode must be 0-20\n");
        return EXIT_FAILURE;
    }

    if (value < 0 || value > 5) {
        printf("Error: Value must be 0-5\n");
        return EXIT_FAILURE;
    }

    int TOUCH_MAGIC = 21504;
    int mode_cmd = (mode == 0 && value == 0) ? 6 : 0;
    int TOUCH_IOC_SETMODE = TOUCH_MAGIC + mode_cmd;
    int arg[2] = {mode, value};

    if (ioctl(fd, TOUCH_IOC_SETMODE, &arg) < 0) {
        printf("Error: Failed to set mode\n");
        return EXIT_FAILURE;
    }

    printf("OK: %s = %d\n", mode_names[mode], value);
    return EXIT_SUCCESS;
}

int apply_profile(int fd, const char *name) {
    int count = sizeof(profiles) / sizeof(profiles[0]);
    
    for (int i = 0; i < count; i++) {
        if (strcmp(profiles[i].name, name) == 0) {
            printf("Applying %s profile...\n", name);
            
            for (int j = 0; j < profiles[i].count; j++) {
                int mode = profiles[i].modes[j][0];
                int value = profiles[i].modes[j][1];
                if (mode >= 0) {
                    if (set_mode(fd, mode, value) != EXIT_SUCCESS) {
                        return EXIT_FAILURE;
                    }
                }
            }
            
            printf("Done!\n");
            return EXIT_SUCCESS;
        }
    }
    
    printf("Error: Unknown profile '%s'\n", name);
    printf("Use 'xiaomi-touch list' to see available profiles\n");
    return EXIT_FAILURE;
}

void show_help() {
    printf("Xiaomi Touch Control v%s\n\n", VERSION);
    
    printf("Usage:\n");
    printf("  xiaomi-touch <profile>        Apply profile\n");
    printf("  xiaomi-touch set <mode> <val> Set mode to value (0-5)\n");
    printf("  xiaomi-touch on <mode>        Turn mode on\n");
    printf("  xiaomi-touch off <mode>       Turn mode off\n");
    printf("  xiaomi-touch list             Show all profiles\n");
    printf("  xiaomi-touch modes            Show all modes\n");
    printf("  xiaomi-touch help             Show this help\n\n");
    
    printf("Quick Examples:\n");
    printf("  xiaomi-touch gaming           # Gaming mode\n");
    printf("  xiaomi-touch normal           # Normal mode\n");
    printf("  xiaomi-touch set 4 5          # Max sensitivity\n");
    printf("  xiaomi-touch on 0             # Game mode on\n");
    printf("  xiaomi-touch off 0            # Game mode off\n\n");    
}

void show_profiles() {
    printf("Available Profiles:\n\n");
    
    int count = sizeof(profiles) / sizeof(profiles[0]);
    for (int i = 0; i < count; i++) {
        printf("  %-12s - %s\n", profiles[i].name, profiles[i].description);
    }
    
    printf("\nUsage: xiaomi-touch <profile>\n");
    printf("Example: xiaomi-touch gaming\n");
}

void show_modes() {
    printf("Touch Modes:\n\n");
    
    for (int i = 0; i < Touch_Mode_NUM; i++) {
        printf("  %2d - %s\n", i, mode_names[i]);
    }
    
    printf("\nUsage:\n");
    printf("  xiaomi-touch set <mode> <value>  # Set specific value (0-5)\n");
    printf("  xiaomi-touch on <mode>           # Turn on\n");
    printf("  xiaomi-touch off <mode>          # Turn off\n\n");
    
    printf("Examples:\n");
    printf("  xiaomi-touch set 4 5    # Max aim sensitivity\n");
    printf("  xiaomi-touch on 0       # Enable game mode\n");
    printf("  xiaomi-touch off 15     # Disable grip mode\n");
}