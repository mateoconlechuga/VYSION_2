#ifndef taskbar_icon_include_file
#define taskbar_icon_include_file

#ifdef __cplusplus
extern "C" {
#endif

#define taskbar_icon_width 32
#define taskbar_icon_height 16
#define taskbar_icon_size 514
#define taskbar_icon ((gfx_sprite_t*)taskbar_icon_data)
extern unsigned char taskbar_icon_data[514];

#ifdef __cplusplus
}
#endif

#endif
