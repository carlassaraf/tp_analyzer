SET(SOURCES components/ui_comp_conttopbar.c
    components/ui_comp.c
    screens/ui_scrBoot.c
    screens/ui_scrPlotter.c
    screens/ui_scrFFT.c
    screens/ui_scrMenu.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_icons_icon_plotter_chart_png.c
    images/ui_img_icons_icon_fft_chart_png.c
    fonts/ui_font_poppinsBold10.c
    fonts/ui_font_poppinsReg10.c
    fonts/ui_font_poppinsReg8.c)

add_library(ui ${SOURCES})
