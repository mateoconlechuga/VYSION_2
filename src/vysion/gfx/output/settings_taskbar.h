#ifndef settings_taskbar_include_file
#define settings_taskbar_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define settings_taskbar_width 16
#define settings_taskbar_height 16
#define settings_taskbar_size 258
#define settings_taskbar ((gfx_sprite_t*)settings_taskbar_data)
extern unsigned char settings_taskbar_data[258];

#ifdef __cplusplus
}
#endif

#endif
