if(CONFIG_ZMK)
  add_subdirectory_ifdef(CONFIG_ZEN_DISPLAY_FULL_REFRESH_PERIOD src)
  add_subdirectory_ifdef(CONFIG_DISPLAY drivers)
endif()
