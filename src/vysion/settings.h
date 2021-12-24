#ifndef SETTINGS_H
#define SETTINGS_H

#include "../optix/colors.h"

#define HASH_SEARCH         0
#define LINEAR_SEARCH       1
#define BINARY_SEARCH       2

#define MAX_PASSWORD_LENGTH 20


struct vysion_settings {
    //os integration
    bool system_on_shortcuts;
    bool system_use_external_editor;
    //VYSION
    bool shell_lock_screen;
    bool shell_use_cursor;
    bool shell_settings_home;
    //filesystem (depracated)
    uint8_t filesystem_search_type;
    uint8_t filesystem_hash_algorithm;
    //colors/customization
    //general
    struct optix_colors customization_colors;
    //desktop
    char customization_wallpaper_name[9];
    bool customization_transparent_taskbar;
    bool customization_centered_taskbar;
    bool customization_display_wallpaper;
    bool customization_display_folders;
    bool customization_display_files;
};

#endif