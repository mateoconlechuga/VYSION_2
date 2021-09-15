#include "settings.h"

void vysion_InitializeSettings(struct vysion_settings *settings) {
    struct vysion_settings default_settings = {
        //os integration
        .os_on_shortcuts = true, 
        .os_use_external_editor = true,
        //VYSION
        .vysion_lock_screen = false,
        .vysion_use_cursor = false,
        .vysion_settings_home = true,
        //filesystem
        .filesystem_search_type = BINARY_SEARCH,
        .filesystem_hash_algorithm = 0,
        //colors/customization
        //general
        //.customization_colors = 
        //desktop
        //customization_wallpaper_name[9];
        .customization_transparent_taskbar = true,
        .customization_centered_taskbar = false,
        .customization_display_wallpaper = true,
        .customization_display_folders = false,
        .customization_display_files = true,

    };



}