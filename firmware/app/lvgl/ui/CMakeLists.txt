SET(SOURCES screens/ui_scrBoot.c
    screens/ui_scrPlotter.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    fonts/ui_font_poppinsBold10.c
    fonts/ui_font_poppinsReg10.c
    fonts/ui_font_poppinsReg8.c)

add_library(ui ${SOURCES})
