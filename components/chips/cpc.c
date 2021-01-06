/*
    cpc.c

    Amstrad CPC 464/6128 and KC Compact. No disc emulation.
*/
/*#include "common/common.h"
#define CHIPS_IMPL
#include "chips/z80.h"
#include "chips/ay38910.h"
#include "chips/i8255.h"
#include "chips/mc6845.h"
#include "chips/am40010.h"
#include "chips/upd765.h"
#include "chips/clk.h"
#include "chips/kbd.h"
#include "chips/mem.h"
#include "chips/fdd.h"
#include "chips/fdd_cpc.h"
#include "systems/cpc.h"
#include "cpc_roms.h"*/
//#include <GL/gl.h>
#include "api.h"

/* imports from cpc-ui.cc */
#ifdef CHIPS_USE_UI
#include "ui.h"
void cpcui_init(cpc_t* cpc);
void cpcui_discard(void);
void cpcui_draw(void);
void cpcui_exec(cpc_t* cpc, uint32_t frame_time_us);
static const int ui_extra_height = 16;
#else
static const int ui_extra_height = 0;
#endif

static cpc_t cpc;
//static cpc_t cpc;

/* sokol-app entry, configure application callbacks and window */
static void app_init(void);
static void app_frame(void);
static void app_input(const sapp_event*);
static void app_cleanup(void);

/*sapp_desc sokol_main(int argc, char* argv[]) {
    sargs_setup(&(sargs_desc){ .argc=argc, .argv=argv });
    return (sapp_desc) {
        .init_cb = app_init,
        .frame_cb = app_frame,
        .event_cb = app_input,
        .cleanup_cb = app_cleanup,
        .width = cpc_std_display_width(),
        .height = 2 * cpc_std_display_height() + ui_extra_height,
        .window_title = "CPC 6128",
        .ios_keyboard_resizes_canvas = true
    };
}*/

cpc_t* readCpcPtr(){
    return &cpc;
}

static unsigned int __wid = 0;

void setRootWid(unsigned int wid){
	__wid = wid;
}

uint64_t readCpc(){
	return cpc.cpu.pins;
	//return cpc;
}

unsigned int getRootWid(){
	return __wid;
}

//void cpcRun(){
	//cpc_run(1,__wid);
	//QFuture<void>  m_f1  = QtConcurrent::run(cpc_run,1,registry.wId());
//}

void cpc_run(int argc, unsigned int wid){
	sapp_desc desc = (sapp_desc) {
        .init_cb = app_init,
        .frame_cb = app_frame,
        .event_cb = app_input,
        .cleanup_cb = app_cleanup,
        .width = cpc_std_display_width(),
        .height = 2 * cpc_std_display_height() + ui_extra_height,
        .window_title = "CPC 6128",
        .ios_keyboard_resizes_canvas = true
    };

    sapp_run2(&desc, wid);
}

//sapp_width(), sapp_height() and sapp_dpi_scale()
/*
SOKOL_API_IMPL int sapp_width(void) {
	return 640;
}

SOKOL_API_IMPL int sapp_height(void) {
	return 480;
}

SOKOL_API_IMPL float sapp_dpi_scale(void) {
	return 1.0;
}*/
//sapp_width      ->
// sapp_height     ->
// sapp_dpi_scale  ->

/* audio-streaming callback */
static void push_audio(const float* samples, int num_samples, void* user_data) {
    (void)user_data;
    saudio_push(samples, num_samples);
}

/* get cpc_desc_t struct based on model and joystick type */
cpc_desc_t cpc_desc(cpc_type_t type, cpc_joystick_type_t joy_type) {
    return (cpc_desc_t) {
        .type = type,
        .joystick_type = joy_type,
        .pixel_buffer = gfx_framebuffer(),
        .pixel_buffer_size = gfx_framebuffer_size(),
        .audio_cb = push_audio,
        .audio_sample_rate = saudio_sample_rate(),
        .rom_464_os = dump_cpc464_os_bin,
        .rom_464_os_size = sizeof(dump_cpc464_os_bin),
        .rom_464_basic = dump_cpc464_basic_bin,
        .rom_464_basic_size = sizeof(dump_cpc464_basic_bin),
        .rom_6128_os = dump_cpc6128_os_bin,
        .rom_6128_os_size = sizeof(dump_cpc6128_os_bin),
        .rom_6128_basic = dump_cpc6128_basic_bin,
        .rom_6128_basic_size = sizeof(dump_cpc6128_basic_bin),
        .rom_6128_amsdos = dump_cpc6128_amsdos_bin,
        .rom_6128_amsdos_size = sizeof(dump_cpc6128_amsdos_bin),
        .rom_kcc_os = dump_kcc_os_bin,
        .rom_kcc_os_size = sizeof(dump_kcc_os_bin),
        .rom_kcc_basic = dump_kcc_bas_bin,
        .rom_kcc_basic_size = sizeof(dump_kcc_bas_bin)
    };
}

/* one-time application init */
void app_init(void) {
    gfx_init(&(gfx_desc_t){
        #ifdef CHIPS_USE_UI
        .draw_extra_cb = ui_draw,
        #endif
        .top_offset = ui_extra_height,
        .aspect_y = 2
    });
    keybuf_init(7);
    clock_init();
    saudio_setup(&(saudio_desc){0});
    fs_init();
    bool delay_input = false;
 /*   if (sargs_exists("file")) {
        delay_input = true;
        if (!fs_load_file(sargs_value("file"))) {
            gfx_flash_error();
        }
    }*/
    cpc_type_t type = CPC_TYPE_6128;
  /*  if (sargs_exists("type")) {
        if (sargs_equals("type", "cpc464")) {
            type = CPC_TYPE_464;
        }
        else if (sargs_equals("type", "kccompact")) {
            type = CPC_TYPE_KCCOMPACT;
        }
    }*/
    cpc_joystick_type_t joy_type = CPC_JOYSTICK_NONE;
 /*   if (sargs_exists("joystick")) {
        joy_type = CPC_JOYSTICK_DIGITAL;
    }*/
    cpc_desc_t desc = cpc_desc(type, joy_type);
    cpc_init(&cpc, &desc);
    #ifdef CHIPS_USE_UI
    cpcui_init(&cpc);
    #endif

    /* keyboard input to send to emulator */
    if (!delay_input) {
     /*   if (sargs_exists("input")) {
            keybuf_put(sargs_value("input"));
        }*/
    }
}

/* per frame stuff, tick the emulator, handle input, decode and draw emulator display */
void app_frame(void) {
    const uint32_t frame_time = clock_frame_time();
    #if CHIPS_USE_UI
        cpcui_exec(&cpc, frame_time);
    #else
        cpc_exec(&cpc, frame_time);
    #endif
    gfx_draw(cpc_display_width(&cpc), cpc_display_height(&cpc));
    const uint32_t load_delay_frames = 120;
    if (fs_ptr() && ((clock_frame_count_60hz() > load_delay_frames) || fs_ext("sna"))) {
        bool load_success = false;
        if (fs_ext("txt") || fs_ext("bas")) {
            load_success = true;
            keybuf_put((const char*)fs_ptr());
        }
        else if (fs_ext("tap")) {
            load_success = cpc_insert_tape(&cpc, fs_ptr(), fs_size());
        }
        else if (fs_ext("dsk")) {
            load_success = cpc_insert_disc(&cpc, fs_ptr(), fs_size());
        }
        else if (fs_ext("sna") || fs_ext("bin")) {
            load_success = cpc_quickload(&cpc, fs_ptr(), fs_size());
        }
        if (load_success) {
            if (clock_frame_count_60hz() > (load_delay_frames + 10)) {
                gfx_flash_success();
            }
            if (sargs_exists("input")) {
                keybuf_put(sargs_value("input"));
            }
        }
        else {
            gfx_flash_error();
        }
        fs_free();
    }
    uint8_t key_code;
    if (0 != (key_code = keybuf_get(frame_time))) {
        cpc_key_down(&cpc, key_code);
        cpc_key_up(&cpc, key_code);
    }
}

/* keyboard input handling */
void app_input(const sapp_event* event) {
    #ifdef CHIPS_USE_UI
    if (ui_input(event)) {
        /* input was handled by UI */
        return;
    }
    #endif
    const bool shift = event->modifiers & SAPP_MODIFIER_SHIFT;
    switch (event->type) {
        int c;
        case SAPP_EVENTTYPE_CHAR:
            c = (int) event->char_code;
            if ((c > 0x20) && (c < 0x7F)) {
                cpc_key_down(&cpc, c);
                cpc_key_up(&cpc, c);
            }
            break;
        case SAPP_EVENTTYPE_KEY_DOWN:
        case SAPP_EVENTTYPE_KEY_UP:
            switch (event->key_code) {
                case SAPP_KEYCODE_SPACE:        c = 0x20; break;
                case SAPP_KEYCODE_LEFT:         c = 0x08; break;
                case SAPP_KEYCODE_RIGHT:        c = 0x09; break;
                case SAPP_KEYCODE_DOWN:         c = 0x0A; break;
                case SAPP_KEYCODE_UP:           c = 0x0B; break;
                case SAPP_KEYCODE_ENTER:        c = 0x0D; break;
                case SAPP_KEYCODE_LEFT_SHIFT:   c = 0x02; break;
                case SAPP_KEYCODE_BACKSPACE:    c = shift ? 0x0C : 0x01; break; // 0x0C: clear screen
                case SAPP_KEYCODE_ESCAPE:       c = shift ? 0x13 : 0x03; break; // 0x13: break
                case SAPP_KEYCODE_F1:           c = 0xF1; break;
                case SAPP_KEYCODE_F2:           c = 0xF2; break;
                case SAPP_KEYCODE_F3:           c = 0xF3; break;
                case SAPP_KEYCODE_F4:           c = 0xF4; break;
                case SAPP_KEYCODE_F5:           c = 0xF5; break;
                case SAPP_KEYCODE_F6:           c = 0xF6; break;
                case SAPP_KEYCODE_F7:           c = 0xF7; break;
                case SAPP_KEYCODE_F8:           c = 0xF8; break;
                case SAPP_KEYCODE_F9:           c = 0xF9; break;
                case SAPP_KEYCODE_F10:          c = 0xFA; break;
                case SAPP_KEYCODE_F11:          c = 0xFB; break;
                case SAPP_KEYCODE_F12:          c = 0xFC; break;
                default:                        c = 0; break;
            }
            if (c) {
                if (event->type == SAPP_EVENTTYPE_KEY_DOWN) {
                    cpc_key_down(&cpc, c);
                }
                else {
                    cpc_key_up(&cpc, c);
                }
            }
            break;
        case SAPP_EVENTTYPE_TOUCHES_BEGAN:
            sapp_show_keyboard(true);
            break;
        default:
            break;
    }
}

/* application cleanup callback */
void app_cleanup(void) {
    cpc_discard(&cpc);
    #ifdef CHIPS_USE_UI
    cpcui_discard();
    #endif
    saudio_shutdown();
    gfx_shutdown();
    //sargs_shutdown();
}
