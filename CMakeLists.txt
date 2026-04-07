if(NOT CONFIG_ZMK)
  return()
endif()

zephyr_library()
zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)

zephyr_library_sources_ifdef(CONFIG_IL0323
  drivers/display/il0323.c
)

if(CONFIG_ZEN_DISPLAY_FULL_REFRESH_PERIOD GREATER 0)
  zephyr_library_sources(src/display_full_refresh.c)
endif()
