#ifndef SETTINGS_H
#define SETTINGS_H

#include "../optix/colors.h"

#define HASH_SEARCH         0
#define LINEAR_SEARCH       1
#define BINARY_SEARCH       2

#define MAX_PASSWORD_LENGTH 20


struct vysion_settings {
    //os integration
    bool os_on_shortcuts : 1;
    bool os_use_external_editor : 1;
    //VYSION
    bool vysion_lock_screen : 1;
    bool vysion_use_cursor : 1;
    bool vysion_settings_home : 1;
    //filesystem
    uint8_t filesystem_search_type : 2;
    uint8_t filesystem_hash_algorithm : 2;
    //colors/customization
    //general
    struct optix_colors customization_colors;
    //desktop
    char customization_wallpaper_name[9];
    bool customization_transparent_taskbar : 1;
    bool customization_centered_taskbar : 1;
    bool customization_display_wallpaper : 1;
    bool customization_display_folders : 1;
    bool customization_display_files : 1;
};

#endif