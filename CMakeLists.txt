# CMake minimum version
cmake_minimum_required (VERSION 3.5)

SET(APPLE FALSE)

# Use the arm-none-eabi-gcc compiler (provide a toolchain file to use something else like clang)
# if (NOT CMAKE_TOOLCHAIN_FILE)
# set(CMAKE_TOOLCHAIN_FILE cmake/arm-gcc.toolchain)
# endif()


# Project Infomation
set(CMAKE_C_COMPILER_FORCED   1)
set(CMAKE_CXX_COMPILER_FORCED 1)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR "arm")


project( VPetCube )
enable_language(ASM)
enable_language(C)


# Reset output path
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

# STDLIB
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)

# Set include path
include_directories (./CORE)
include_directories (./FreeRTOS/include)
include_directories (./FWLIB/inc)
include_directories (./SYSTEM/delay)
include_directories (./SYSTEM/sys)
include_directories (./SYSTEM/usart)
include_directories (./Middlewares/LVGL/guiguider)
include_directories (./Middlewares/LVGL/guiguider/guider_fonts)
include_directories (./Middlewares/LVGL/guiguider/guider_customer_fonts)
include_directories (./Middlewares/LVGL/GUI_APP)
include_directories (./Middlewares/LVGL/GUI/lvgl/examples/porting)
include_directories (./Middlewares/LVGL/GUI/lvgl/src/extra)
include_directories (./Middlewares/LVGL/GUI/lvgl/src/extra/libs)
include_directories (./Middlewares/LVGL/GUI/lvgl/src/extra/libs/fsdrv)
include_directories (./Middlewares/LVGL/GUI/lvgl/src)
include_directories (./Middlewares/LVGL/GUI/lvgl)
include_directories(./Middlewares/FATFS)
include_directories(./Middlewares/FATFS/App)
include_directories(./Middlewares/FATFS/Target)
include_directories (./HARDWARE)
include_directories (./HARDWARE/ADC)
include_directories (./HARDWARE/ESP8266)
include_directories (./HARDWARE/EXTI)
include_directories (./HARDWARE/IIC)
include_directories (./HARDWARE/LCD)
include_directories (./HARDWARE/MPU6050)
include_directories (./HARDWARE/MPU6050/eMPL)
include_directories (./HARDWARE/SDIO)
include_directories (./HARDWARE/RTC)
include_directories (./HARDWARE/TIMER)
include_directories (./HARDWARE/SDIO)
include_directories (./HARDWARE/USART2)
include_directories (./USER)
include_directories (./FreeRTOS/portable/GCC/ARM_CM4F)
include_directories (./FreeRTOS/portable/MemMang)

# The need build source path and build all files
set (SRC_FILE0 ./USER/main.c)
set (SRC_FILE1 ./USER/stm32f4xx_it.c)
set (SRC_FILE2 ./USER/system_stm32f4xx.c)
set (SRC_FILE3 ./FreeRTOS/include/FreeRTOSConfig.h)
set (SRC_FILE4 ./HARDWARE/MPU6050/mpu6050.c)
set (SRC_FILE5 ./HARDWARE/MPU6050/mpu6050.h)
set (SRC_FILE6 ./HARDWARE/IIC/myiic.c)
set (SRC_FILE7 ./HARDWARE/IIC/myiic.h)
set (SRC_FILE8 ./HARDWARE/MPU6050/eMPL/inv_mpu.c)
set (SRC_FILE9 ./HARDWARE/MPU6050/eMPL/inv_mpu.h)
set (SRC_FILE10 ./HARDWARE/MPU6050/eMPL/inv_mpu_dmp_motion_driver.c)
set (SRC_FILE11 ./HARDWARE/MPU6050/eMPL/inv_mpu_dmp_motion_driver.h)
set (SRC_FILE12 ./HARDWARE/USART2/usart2.c)
set (SRC_FILE13 ./HARDWARE/USART2/usart2.h)
set (SRC_FILE14 ./HARDWARE/TIMER/timer.c)
set (SRC_FILE15 ./HARDWARE/TIMER/timer.h)
set (SRC_FILE16 ./HARDWARE/EXTI/exti.c)
set (SRC_FILE17 ./HARDWARE/EXTI/exti.h)
set (SRC_FILE18 ./HARDWARE/ESP8266/esp8266_common.c)
set (SRC_FILE19 ./HARDWARE/ESP8266/esp8266_common.h)
set (SRC_FILE20 ./HARDWARE/base_function.c)
set (SRC_FILE21 ./HARDWARE/base_function.h)
set (SRC_FILE22 ./HARDWARE/RTC/rtc.c)
set (SRC_FILE23 ./HARDWARE/RTC/rtc.h)
set (SRC_FILE24 ./HARDWARE/LCD/lcd.c)
set (SRC_FILE25 ./HARDWARE/LCD/lcd.h)
set (SRC_FILE26 ./HARDWARE/LCD/lcd_dma.c)
set (SRC_FILE27 ./HARDWARE/LCD/lcd_dma.h)
set (SRC_FILE28 ./HARDWARE/LCD/lcd_init.c)
set (SRC_FILE29 ./HARDWARE/LCD/lcd_init.h)
set (SRC_FILE30 ./HARDWARE/LCD/lcd_spi.c)
set (SRC_FILE31 ./HARDWARE/LCD/lcd_spi.h)
set (SRC_FILE32 ./HARDWARE/LCD/lcdfont.h)
set (SRC_FILE33 ./HARDWARE/ADC/adc.c)
set (SRC_FILE34 ./HARDWARE/ADC/adc.h)
set (SRC_FILE35 ./SYSTEM/delay/delay.c)
set (SRC_FILE36 ./SYSTEM/sys/sys.c)
set (SRC_FILE37 ./SYSTEM/usart/usart.c)
set (SRC_FILE38 ./SYSTEM/sys/sys.h)
set (SRC_FILE39 ./FWLIB/src/misc.c)
set (SRC_FILE40 ./FWLIB/src/stm32f4xx_gpio.c)
set (SRC_FILE41 ./FWLIB/src/stm32f4xx_rcc.c)
set (SRC_FILE42 ./FWLIB/src/stm32f4xx_syscfg.c)
set (SRC_FILE43 ./FWLIB/src/stm32f4xx_usart.c)
set (SRC_FILE44 ./FWLIB/src/stm32f4xx_tim.c)
set (SRC_FILE45 ./FWLIB/src/stm32f4xx_exti.c)
set (SRC_FILE46 ./FWLIB/src/stm32f4xx_rtc.c)
set (SRC_FILE47 ./FWLIB/src/stm32f4xx_pwr.c)
set (SRC_FILE48 ./FWLIB/src/stm32f4xx_spi.c)
set (SRC_FILE49 ./FWLIB/src/stm32f4xx_dma.c)
set (SRC_FILE50 ./FWLIB/src/stm32f4xx_adc.c)
set (SRC_FILE51 ./FreeRTOS/croutine.c)
set (SRC_FILE52 ./FreeRTOS/event_groups.c)
set (SRC_FILE53 ./FreeRTOS/list.c)
set (SRC_FILE54 ./FreeRTOS/queue.c)
set (SRC_FILE55 ./FreeRTOS/tasks.c)
set (SRC_FILE56 ./FreeRTOS/timers.c)
set (SRC_FILE57 ./FreeRTOS/portable/GCC/ARM_CM4F/port.c)
set (SRC_FILE58 ./FreeRTOS/portable/MemMang/heap_4.c)
set (SRC_FILE59 ./Middlewares/LVGL/GUI_APP/app_window.c)
set (SRC_FILE60 ./Middlewares/LVGL/GUI_APP/app_window.h)
set (SRC_FILE61 ./Middlewares/LVGL/GUI_APP/img_lvgl_logo.c)
set (SRC_FILE62 ./Middlewares/LVGL/guiguider/gui_guider.c)
set (SRC_FILE63 ./Middlewares/LVGL/guiguider/gui_guider.h)
set (SRC_FILE64 ./Middlewares/LVGL/guiguider/setup_scr_init.c)
set (SRC_FILE65 ./Middlewares/LVGL/guiguider/setup_scr_deskop.c)
set (SRC_FILE66 ./Middlewares/LVGL/guiguider/guider_fonts/lv_font_montserratMedium_20.c)
set (SRC_FILE67 ./Middlewares/LVGL/guiguider/guider_fonts/lv_font_montserratMedium_70.c)
set (SRC_FILE68 ./Middlewares/LVGL/guiguider/guider_fonts/lv_font_simsun_12.c)
set (SRC_FILE69 ./Middlewares/LVGL/guiguider/guider_fonts/lv_font_simsun_20.c)
set (SRC_FILE70 ./Middlewares/LVGL/guiguider/guider_fonts/lv_font_simsun_40.c)
set (SRC_FILE71 ./Middlewares/LVGL/guiguider/images/dancePattern.c)
set (SRC_FILE72 ./Middlewares/LVGL/guiguider/dance_pic_data.c)
set (SRC_FILE73 ./Middlewares/LVGL/guiguider/dance_pic_data.h)
set (SRC_FILE74 ./Middlewares/LVGL/guiguider/images/weatherPattern.c)
set (SRC_FILE75 ./Middlewares/LVGL/guiguider/weather_pic_data.c)
set (SRC_FILE76 ./Middlewares/LVGL/guiguider/weather_pic_data.h)
set (SRC_FILE77 ./Middlewares/LVGL/GUI/lvgl/examples/porting/lv_port_disp_template.c)
set (SRC_FILE78 .//Middlewares/LVGL/GUI/lvgl/examples/porting/lv_port_indev_template.c)
set (SRC_FILE79 .//Middlewares/LVGL/GUI/lvgl/lv_conf.h)
set (SRC_FILE80 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_disp.c)
set (SRC_FILE81 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_event.c)
set (SRC_FILE82 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_group.c)
set (SRC_FILE83 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_indev.c)
set (SRC_FILE84 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_indev_scroll.c)
set (SRC_FILE85 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_obj.c)
set (SRC_FILE86 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_class.c)
set (SRC_FILE87 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_draw.c)
set (SRC_FILE88 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_pos.c)
set (SRC_FILE89 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_scroll.c)
set (SRC_FILE90 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style.c)
set (SRC_FILE91 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_style_gen.c)
set (SRC_FILE92 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_obj_tree.c)
set (SRC_FILE93 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_refr.c)
set (SRC_FILE94 .//Middlewares/LVGL/GUI/lvgl/src/core/lv_theme.c)
set (SRC_FILE95 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw.c)
set (SRC_FILE96 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_arc.c)
set (SRC_FILE97 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_img.c)
set (SRC_FILE98 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_label.c)
set (SRC_FILE99 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_line.c)
set (SRC_FILE100 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_mask.c)
set (SRC_FILE101 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_rect.c)
set (SRC_FILE102 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_draw_triangle.c)
set (SRC_FILE103 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_buf.c)
set (SRC_FILE104 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_cache.c)
set (SRC_FILE105 .//Middlewares/LVGL/GUI/lvgl/src/draw/lv_img_decoder.c)
set (SRC_FILE106 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw.c)
set (SRC_FILE107 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_arc.c)
set (SRC_FILE108 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_blend.c)
set (SRC_FILE109 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_dither.c)
set (SRC_FILE110 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_gradient.c)
set (SRC_FILE111 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_img.c)
set (SRC_FILE112 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_letter.c)
set (SRC_FILE113 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_line.c)
set (SRC_FILE114 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_polygon.c)
set (SRC_FILE115 .//Middlewares/LVGL/GUI/lvgl/src/draw/sw/lv_draw_sw_rect.c)
set (SRC_FILE116 .//Middlewares/LVGL/GUI/lvgl/src/extra/lv_extra.c)
set (SRC_FILE117 .//Middlewares/LVGL/GUI/lvgl/src/extra/layouts/flex/lv_flex.c)
set (SRC_FILE118 .//Middlewares/LVGL/GUI/lvgl/src/extra/layouts/grid/lv_grid.c)
set (SRC_FILE119 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/bmp/lv_bmp.c)
set (SRC_FILE120 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/ffmpeg/lv_ffmpeg.c)
set (SRC_FILE121 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/freetype/lv_freetype.c)
set (SRC_FILE122 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/fsdrv/lv_fs_fatfs.c)
set (SRC_FILE123 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/fsdrv/lv_fs_posix.c)
set (SRC_FILE124 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/fsdrv/lv_fs_stdio.c)
set (SRC_FILE125 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/fsdrv/lv_fs_win32.c)
set (SRC_FILE126 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/gif/gifdec.c)
set (SRC_FILE127 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/gif/lv_gif.c)
set (SRC_FILE128 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/png/lodepng.c)
set (SRC_FILE129 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/png/lv_png.c)
set (SRC_FILE130 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/qrcode/lv_qrcode.c)
set (SRC_FILE131 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/qrcode/qrcodegen.c)
set (SRC_FILE132 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/rlottie/lv_rlottie.c)
set (SRC_FILE133 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/sjpg/lv_sjpg.c)
set (SRC_FILE134 .//Middlewares/LVGL/GUI/lvgl/src/extra/libs/sjpg/tjpgd.c)
set (SRC_FILE135 .//Middlewares/LVGL/GUI/lvgl/src/extra/others/gridnav/lv_gridnav.c)
set (SRC_FILE136 .//Middlewares/LVGL/GUI/lvgl/src/extra/others/monkey/lv_monkey.c)
set (SRC_FILE137 .//Middlewares/LVGL/GUI/lvgl/src/extra/others/snapshot/lv_snapshot.c)
set (SRC_FILE138 .//Middlewares/LVGL/GUI/lvgl/src/extra/themes/basic/lv_theme_basic.c)
set (SRC_FILE139 .//Middlewares/LVGL/GUI/lvgl/src/extra/themes/default/lv_theme_default.c)
set (SRC_FILE140 .//Middlewares/LVGL/GUI/lvgl/src/extra/themes/mono/lv_theme_mono.c)
set (SRC_FILE141 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/animimg/lv_animimg.c)
set (SRC_FILE142 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/calendar/lv_calendar.c)
set (SRC_FILE143 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/calendar/lv_calendar_header_arrow.c)
set (SRC_FILE144 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/calendar/lv_calendar_header_dropdown.c)
set (SRC_FILE145 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/chart/lv_chart.c)
set (SRC_FILE146 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/colorwheel/lv_colorwheel.c)
set (SRC_FILE147 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/imgbtn/lv_imgbtn.c)
set (SRC_FILE148 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/keyboard/lv_keyboard.c)
set (SRC_FILE149 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/led/lv_led.c)
set (SRC_FILE150 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/list/lv_list.c)
set (SRC_FILE151 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/menu/lv_menu.c)
set (SRC_FILE152 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/meter/lv_meter.c)
set (SRC_FILE153 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/msgbox/lv_msgbox.c)
set (SRC_FILE154 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/span/lv_span.c)
set (SRC_FILE155 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/spinbox/lv_spinbox.c)
set (SRC_FILE156 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/spinner/lv_spinner.c)
set (SRC_FILE157 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/tabview/lv_tabview.c)
set (SRC_FILE158 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/tileview/lv_tileview.c)
set (SRC_FILE159 .//Middlewares/LVGL/GUI/lvgl/src/extra/widgets/win/lv_win.c)
set (SRC_FILE160 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font.c)
set (SRC_FILE161 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_dejavu_16_persian_hebrew.c)
set (SRC_FILE162 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_fmt_txt.c)
set (SRC_FILE163 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_loader.c)
set (SRC_FILE164 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_8.c)
set (SRC_FILE165 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_10.c)
set (SRC_FILE166 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_12.c)
set (SRC_FILE167 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_12_subpx.c)
set (SRC_FILE168 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_14.c)
set (SRC_FILE169 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_16.c)
set (SRC_FILE170 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_18.c)
set (SRC_FILE171 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_20.c)
set (SRC_FILE172 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_22.c)
set (SRC_FILE173 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_24.c)
set (SRC_FILE174 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_26.c)
set (SRC_FILE175 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_28.c)
set (SRC_FILE176 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_28_compressed.c)
set (SRC_FILE177 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_30.c)
set (SRC_FILE178 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_32.c)
set (SRC_FILE179 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_34.c)
set (SRC_FILE180 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_36.c)
set (SRC_FILE181 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_38.c)
set (SRC_FILE182 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_40.c)
set (SRC_FILE183 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_42.c)
set (SRC_FILE184 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_44.c)
set (SRC_FILE185 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_46.c)
set (SRC_FILE186 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_montserrat_48.c)
set (SRC_FILE187 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_simsun_16_cjk.c)
set (SRC_FILE188 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_unscii_8.c)
set (SRC_FILE189 .//Middlewares/LVGL/GUI/lvgl/src/font/lv_font_unscii_16.c)
set (SRC_FILE190 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl.c)
set (SRC_FILE191 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_arc.c)
set (SRC_FILE192 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_bg.c)
set (SRC_FILE193 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_composite.c)
set (SRC_FILE194 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_img.c)
set (SRC_FILE195 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_label.c)
set (SRC_FILE196 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_line.c)
set (SRC_FILE197 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_mask.c)
set (SRC_FILE198 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_polygon.c)
set (SRC_FILE199 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_rect.c)
set (SRC_FILE200 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.c)
set (SRC_FILE201 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.c)
set (SRC_FILE202 .//Middlewares/LVGL/GUI/lvgl/src/draw/sdl/lv_draw_sdl_utils.c)
set (SRC_FILE203 .//Middlewares/LVGL/GUI/lvgl/src/draw/stm32_dma2d/lv_gpu_stm32_dma2d.c)
set (SRC_FILE204 .//Middlewares/LVGL/GUI/lvgl/src/hal/lv_hal_disp.c)
set (SRC_FILE205 .//Middlewares/LVGL/GUI/lvgl/src/hal/lv_hal_indev.c)
set (SRC_FILE206 .//Middlewares/LVGL/GUI/lvgl/src/hal/lv_hal_tick.c)
set (SRC_FILE207 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim.c)
set (SRC_FILE208 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_anim_timeline.c)
set (SRC_FILE209 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_area.c)
set (SRC_FILE210 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_async.c)
set (SRC_FILE211 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_bidi.c)
set (SRC_FILE212 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_color.c)
set (SRC_FILE213 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_fs.c)
set (SRC_FILE214 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_gc.c)
set (SRC_FILE215 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_ll.c)
set (SRC_FILE216 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_log.c)
set (SRC_FILE217 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_lru.c)
set (SRC_FILE218 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_math.c)
set (SRC_FILE219 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_mem.c)
set (SRC_FILE220 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_printf.c)
set (SRC_FILE221 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_style.c)
set (SRC_FILE222 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_style_gen.c)
set (SRC_FILE223 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_templ.c)
set (SRC_FILE224 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_timer.c)
set (SRC_FILE225 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_tlsf.c)
set (SRC_FILE226 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt.c)
set (SRC_FILE227 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_txt_ap.c)
set (SRC_FILE228 .//Middlewares/LVGL/GUI/lvgl/src/misc/lv_utils.c)
set (SRC_FILE229 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_arc.c)
set (SRC_FILE230 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_bar.c)
set (SRC_FILE231 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btn.c)
set (SRC_FILE232 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_btnmatrix.c)
set (SRC_FILE233 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_canvas.c)
set (SRC_FILE234 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_checkbox.c)
set (SRC_FILE235 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_dropdown.c)
set (SRC_FILE236 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_img.c)
set (SRC_FILE237 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_label.c)
set (SRC_FILE238 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_line.c)
set (SRC_FILE239 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_objx_templ.c)
set (SRC_FILE240 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_roller.c)
set (SRC_FILE241 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_slider.c)
set (SRC_FILE242 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_switch.c)
set (SRC_FILE243 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_table.c)
set (SRC_FILE244 .//Middlewares/LVGL/GUI/lvgl/src/widgets/lv_textarea.c)
set(SRC_FILE245 .//Middlewares/FATFS/App/fatfs.c)
set(SRC_FILE247 .//Middlewares/FATFS/App/fatfs.h)
set(SRC_FILE248 .//Middlewares/FATFS/Target/ffconf.h)
set(SRC_FILE249 .//Middlewares/FATFS/Target/diskio.c)
set(SRC_FILE250 .//Middlewares/FATFS/Target/diskio.h)
set(SRC_FILE251 .//Middlewares/FATFS/Target/fatfs_platform.h)
set(SRC_FILE251 .//Middlewares/FATFS/Target/fatfs_platform.c)
set(SRC_FILE251 .//Middlewares/FATFS/Target/bsp_driver_sd.c)
set(SRC_FILE251 .//Middlewares/FATFS/Target/bsp_driver_sd.h)
set(SRC_FILE253 .//HARDWARE/SDIO/sdio.c)
set (SRC_FILE254 .//HARDWARE/SDIO/sdio.h)
set(CROSS_TARGET_TRIPLET "arm-none-eabi-")

# CC AR LD AS
set(CMAKE_C_COMPILER "${CROSS_TARGET_TRIPLET}gcc")
set(CMAKE_ASM_COMPILER "${CROSS_TARGET_TRIPLET}gcc")

# CFLAGS
set (CMAKE_C_FLAGS "-g -Wextra -Wshadow -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -fno-common -ffunction-sections -fdata-sections -MD -Wall -Wundef -mfloat-abi=hard -mthumb -mcpu=cortex-m4 " CACHE INTERNAL "c compiler flags")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -D STM32F40X_HD  -D USE_STDPERIPH_DRIVER ")

# CXXFLAGS
set (CMAKE_CXX_FLAGS "-Wextra -Wshadow -Wredundant-decls  -Weffc++ -fno-common -ffunction-sections -fdata-sections -MD -Wall -Wundef -mthumb -mcpu=cortex-m4 " CACHE INTERNAL "cxx compiler flags")

# ASMFLAGS
set (CMAKE_ASM_FLAGS "-g -mthumb -mcpu=cortex-m4 " CACHE INTERNAL "asm compiler flags")

# LDFLAGS
set (CMAKE_EXE_LINKER_FLAGS "-g -Wl,--gc-sections -Wl,-Map=Holocubic.map -mthumb -mcpu=cortex-m4 " CACHE INTERNAL "executable linker flags")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_SOURCE_DIR}/STM32FLASH.ld -L ")

if(NOT DEFINED OPENOCD_CFG)
	find_file(OPENOCD_CFG_FILE "openocd.cfg" "." "./src" "${CMAKE_BINARY_DIR}"
		NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)

	if(OPENOCD_CFG_FILE)
		set(OPENOCD_CFG -f ${OPENOCD_CFG_FILE})
		message("Found openocd.cfg: ${OPENOCD_CFG_FILE}")
	else()
		set(OPENOCD_CFG -f interface/stlink-v2.cfg -f target/${stm32_family}x.cfg)
	endif()
endif()

if(NOT DEFINED MCU_STARTUP_FILE)
	set(MCU_STARTUP_SEARCH_DIRS"." "./MDK-ARM" "./Startup" "./Startup")
	message("Looking for Startup Assembly in ${MCU_STARTUP_SEARCH_DIRS}")
	find_file(MCU_STARTUP_FILE "startup_stm32f405rgtx.s" MCU_STARTUP_SEARCH_DIRS
		NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
endif()

if(MCU_STARTUP_FILE)
	message("MCU_STARTUP_FILE: ${MCU_STARTUP_FILE}")
else()
	message(FATAL_ERROR "Could not find a startup file")
endif()

# set(MEMMAP_SEARCH_DIRS
# 	"." "./MDK-ARM"
# 	"./src/mem/${USE_COMPILER}" "./src/mem")

# if(NOT LINKER_SCRIPT AND USE_COMPILER STREQUAL "GCC")
# 	message("Looking for Linker Script (GCC)")
# 	find_file(LINKER_SCRIPT "${STM32FXXX}.ld" ${MEMMAP_SEARCH_DIRS}
# 		NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
# 	find_file(LINKER_SCRIPT "${STM32_SERIES_B}.ld" ${MEMMAP_SEARCH_DIRS}
# 		NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
# 	find_file(LINKER_SCRIPT "${STM32_SERIES_A}.ld" ${MEMMAP_SEARCH_DIRS}
# 		NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
# 	find_file(LINKER_SCRIPT "${STM32_SERIES}.ld" ${MEMMAP_SEARCH_DIRS}
# 		NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
# 	find_file(LINKER_SCRIPT "${STM32_FAMILY}.ld" ${MEMMAP_SEARCH_DIRS}
# 		NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
# 	find_file(LINKER_SCRIPT "${STM32FXXX}TX_FLASH.ld" ${MEMMAP_SEARCH_DIRS}
# 		NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH NO_SYSTEM_ENVIRONMENT_PATH)
# endif()

# if(NOT LINKER_SCRIPT AND USE_COMPILER STREQUAL "GCC")
# 	message(FATAL_ERROR "Could not find a linker script")
# else()
# 	message("LINKER_SCRIPT: ${LINKER_SCRIPT}")
# endif()

if(NOT DEFINED XTAL)
	set(XTAL 8000000)
endif()



# Generate the target
add_executable (${CMAKE_PROJECT_NAME}.elf  ${SRC_FILE0}  ${SRC_FILE1}  ${SRC_FILE2}  ${SRC_FILE3}  ${SRC_FILE4}  ${SRC_FILE5}  ${SRC_FILE6}  ${SRC_FILE7}  ${SRC_FILE8}  ${SRC_FILE9}  ${SRC_FILE10}  ${SRC_FILE11}  ${SRC_FILE12}  ${SRC_FILE13}  ${SRC_FILE14}  ${SRC_FILE15}  ${SRC_FILE16}  ${SRC_FILE17}  ${SRC_FILE18}  ${SRC_FILE19}  ${SRC_FILE20}  ${SRC_FILE21}  ${SRC_FILE22}  ${SRC_FILE23}  ${SRC_FILE24}  ${SRC_FILE25}  ${SRC_FILE26}  ${SRC_FILE27}  ${SRC_FILE28}  ${SRC_FILE29}  ${SRC_FILE30}  ${SRC_FILE31}  ${SRC_FILE32}  ${SRC_FILE33}  ${SRC_FILE34}  ${SRC_FILE35}  ${SRC_FILE36}  ${SRC_FILE37}  ${SRC_FILE38}  ${SRC_FILE39}  ${SRC_FILE40}  ${SRC_FILE41}  ${SRC_FILE42}  ${SRC_FILE43}  ${SRC_FILE44}  ${SRC_FILE45}  ${SRC_FILE46}  ${SRC_FILE47}  ${SRC_FILE48}  ${SRC_FILE49}  ${SRC_FILE50}  ${SRC_FILE51}  ${SRC_FILE52}  ${SRC_FILE53}  ${SRC_FILE54}  ${SRC_FILE55}  ${SRC_FILE56}  ${SRC_FILE57}  ${SRC_FILE58}  ${SRC_FILE59}  ${SRC_FILE60}  ${SRC_FILE61}  ${SRC_FILE62}  ${SRC_FILE63}  ${SRC_FILE64}  ${SRC_FILE65}  ${SRC_FILE66}  ${SRC_FILE67}  ${SRC_FILE68}  ${SRC_FILE69}  ${SRC_FILE70}  ${SRC_FILE71}  ${SRC_FILE72}  ${SRC_FILE73}  ${SRC_FILE74}  ${SRC_FILE75}  ${SRC_FILE76}  ${SRC_FILE77}  ${SRC_FILE78}  ${SRC_FILE79}  ${SRC_FILE80}  ${SRC_FILE81}  ${SRC_FILE82}  ${SRC_FILE83}  ${SRC_FILE84}  ${SRC_FILE85}  ${SRC_FILE86}  ${SRC_FILE87}  ${SRC_FILE88}  ${SRC_FILE89}  ${SRC_FILE90}  ${SRC_FILE91}  ${SRC_FILE92}  ${SRC_FILE93}  ${SRC_FILE94}  ${SRC_FILE95}  ${SRC_FILE96}  ${SRC_FILE97}  ${SRC_FILE98}  ${SRC_FILE99}  ${SRC_FILE100}  ${SRC_FILE101}  ${SRC_FILE102}  ${SRC_FILE103}  ${SRC_FILE104}  ${SRC_FILE105}  ${SRC_FILE106}  ${SRC_FILE107}  ${SRC_FILE108}  ${SRC_FILE109}  ${SRC_FILE110}  ${SRC_FILE111}  ${SRC_FILE112}  ${SRC_FILE113}  ${SRC_FILE114}  ${SRC_FILE115}  ${SRC_FILE116}  ${SRC_FILE117}  ${SRC_FILE118}  ${SRC_FILE119}  ${SRC_FILE120}  ${SRC_FILE121}  ${SRC_FILE122}  ${SRC_FILE123}  ${SRC_FILE124}  ${SRC_FILE125}  ${SRC_FILE126}  ${SRC_FILE127}  ${SRC_FILE128}  ${SRC_FILE129}  ${SRC_FILE130}  ${SRC_FILE131}  ${SRC_FILE132}  ${SRC_FILE133}  ${SRC_FILE134}  ${SRC_FILE135}  ${SRC_FILE136}  ${SRC_FILE137}  ${SRC_FILE138}  ${SRC_FILE139}  ${SRC_FILE140}  ${SRC_FILE141}  ${SRC_FILE142}  ${SRC_FILE143}  ${SRC_FILE144}  ${SRC_FILE145}  ${SRC_FILE146}  ${SRC_FILE147}  ${SRC_FILE148}  ${SRC_FILE149}  ${SRC_FILE150}  ${SRC_FILE151}  ${SRC_FILE152}  ${SRC_FILE153}  ${SRC_FILE154}  ${SRC_FILE155}  ${SRC_FILE156}  ${SRC_FILE157}  ${SRC_FILE158}  ${SRC_FILE159}  ${SRC_FILE160}  ${SRC_FILE161}  ${SRC_FILE162}  ${SRC_FILE163}  ${SRC_FILE164}  ${SRC_FILE165}  ${SRC_FILE166}  ${SRC_FILE167}  ${SRC_FILE168}  ${SRC_FILE169}  ${SRC_FILE170}  ${SRC_FILE171}  ${SRC_FILE172}  ${SRC_FILE173}  ${SRC_FILE174}  ${SRC_FILE175}  ${SRC_FILE176}  ${SRC_FILE177}  ${SRC_FILE178}  ${SRC_FILE179}  ${SRC_FILE180}  ${SRC_FILE181}  ${SRC_FILE182}  ${SRC_FILE183}  ${SRC_FILE184}  ${SRC_FILE185}  ${SRC_FILE186}  ${SRC_FILE187}  ${SRC_FILE188}  ${SRC_FILE189}  ${SRC_FILE190}  ${SRC_FILE191}  ${SRC_FILE192}  ${SRC_FILE193}  ${SRC_FILE194}  ${SRC_FILE195}  ${SRC_FILE196}  ${SRC_FILE197}  ${SRC_FILE198}  ${SRC_FILE199}  ${SRC_FILE200}  ${SRC_FILE201}  ${SRC_FILE202}  ${SRC_FILE203}  ${SRC_FILE204}  ${SRC_FILE205}  ${SRC_FILE206}  ${SRC_FILE207}  ${SRC_FILE208}  ${SRC_FILE209}  ${SRC_FILE210}  ${SRC_FILE211}  ${SRC_FILE212}  ${SRC_FILE213}  ${SRC_FILE214}  ${SRC_FILE215}  ${SRC_FILE216}  ${SRC_FILE217}  ${SRC_FILE218}  ${SRC_FILE219}  ${SRC_FILE220}  ${SRC_FILE221}  ${SRC_FILE222}  ${SRC_FILE223}  ${SRC_FILE224}  ${SRC_FILE225}  ${SRC_FILE226}  ${SRC_FILE227}  ${SRC_FILE228}  ${SRC_FILE229}  ${SRC_FILE230}  ${SRC_FILE231}  ${SRC_FILE232}  ${SRC_FILE233}  ${SRC_FILE234}  ${SRC_FILE235}  ${SRC_FILE236}  ${SRC_FILE237}  ${SRC_FILE238}  ${SRC_FILE239}  ${SRC_FILE240}  ${SRC_FILE241}  ${SRC_FILE242}  ${SRC_FILE243}  ${SRC_FILE244} )

# Link the library to the target
target_link_libraries (${CMAKE_PROJECT_NAME}.elf )

# Generate the binary file
add_custom_target (${CMAKE_PROJECT_NAME}.bin ALL arm-none-eabi-objcopy -Obinary "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)

# Generate the hex file
add_custom_target (${CMAKE_PROJECT_NAME}.hex ALL arm-none-eabi-objcopy -Oihex "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.hex" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)

# Echo the size Infomation
add_custom_target (size ALL arm-none-eabi-size "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf" DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.elf)

# Make flash to the board by st-link
# add_custom_target (flash COMMAND st-flash write ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_PROJECT_NAME}.bin 0x8000000)
find_program(OPENOCD_EXE "openocd")


# function(stlinkutil_targets)
# 	if(STLINKUTIL)
# 		add_custom_target(flash_stlinkutil
# 			${STLINKUTIL} -c -P ${PROJECT_NAME}.hex -Rst -Run
# 			DEPENDS ${PROJECT_NAME}
# 			COMMENT "Flashing target hardware (by STLink Utility CLI)")

# 		add_custom_target(erase_stlinkutil
# 			${STLINKUTIL} -c -ME
# 			COMMENT "Mass Erase all flash sectors")

# 		set(FLASH_TARGET flash_stlinkutil PARENT_SCOPE)
# 		set(ERASE_TARGET erase_stlinkutil PARENT_SCOPE)
# 	endif()
# endfunction()

# function(stflash_targets)
# 	if(STFLASH_EXE)
# 		add_custom_target(flash_stflash
# 			${STFLASH_EXE} --reset --format ihex write ${PROJECT_NAME}.hex
# 			DEPENDS ${PROJECT_NAME}
# 			COMMENT "Flashing target hardware (by Texane/STlink) (Remember to reset to run the program)")

# 		add_custom_target(erase_stflash
# 			${STFLASH_EXE} erase
# 			COMMENT "Mass Erase all flash sectors")

# 		find_program(STUTIL "st-util")

# 		if(STUTIL)
# 			add_custom_target(debug_stutil
# 				${STUTIL} -m -p 3333
# 				DEPENDS ${PROJECT_NAME}
# 				COMMENT "Running ST-UTIL as a GDB host (port 3333)")
# 			set(DEBUG_TARGET debug_stutil PARENT_SCOPE)
# 		endif()

# 		set(FLASH_TARGET flash_stflash PARENT_SCOPE)
# 		set(ERASE_TARGET erase_stflash PARENT_SCOPE)
# 	endif()
# endfunction()

function(openocd_targets)
	if(OPENOCD_EXE)
		add_custom_target(flash_openocd
			${OPENOCD_EXE} -j4 -c "transport select hla_swd"
			-c "init" -c "reset halt"
			-c "flash write_image erase ${PROJECT_NAME}.elf" -c "reset run" -c "shutdown"
			DEPENDS ${PROJECT_NAME}
			COMMENT "Flashing target hardware (by OpenOCD)")

		add_custom_target(debug_openocd
			${OPENOCD_EXE} -j4
			DEPENDS ${PROJECT_NAME}
			COMMENT "Running OpenOCD as a GDB host (port 3333)")

		add_custom_target(gdb_openocd
			gdbgui -g arm-none-eabi-gdb --args="${PROJECT_NAME}.elf")

		add_custom_target(erase_openocd
			${OPENOCD_EXE} -j4 -c "transport select hla_swd"
			-c "init" -c "reset halt"
			-c "flash erase_sector 0 0 last" -c "shutdown"
			DEPENDS ${PROJECT_NAME}
			COMMENT "Mass Erase all flash sectors")

		set(FLASH_TARGET flash_openocd PARENT_SCOPE)
		set(ERASE_TARGET erase_openocd PARENT_SCOPE)
		set(DEBUG_TARGET debug_openocd PARENT_SCOPE)
		set(GDB_TARGET gdb_openocd PARENT_SCOPE)
	endif()
endfunction()


if(DEFINED FLASH_TARGET)
	add_custom_target(flash DEPENDS ${FLASH_TARGET})
	message("flash using ${FLASH_TARGET}")
else()
	message("No program found for flash")
endif()

if(DEFINED ERASE_TARGET)
	add_custom_target(erase DEPENDS ${ERASE_TARGET})
	message("erase using ${ERASE_TARGET}")
else()
	message("No program found for erase")
endif()

if(DEFINED DEBUG_TARGET)
	add_custom_target(debug DEPENDS ${DEBUG_TARGET})
	message("debug using ${DEBUG_TARGET}")
else()
	message("No program found for debug")
endif()

if(DEFINED GDB_TARGET)
	add_custom_target(gdb DEPENDS ${GDB_TARGET})
	message("gdb using ${GDB_TARGET}")
else()
	message("No program found for gdb")
endif()

add_custom_target (clean-all COMMAND rm -rf ${CMAKE_BINARY_DIR}/*)
