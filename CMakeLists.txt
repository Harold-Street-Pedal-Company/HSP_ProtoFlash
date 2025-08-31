project(HSP_ProtoFlash VERSION 1.0.0 LANGUAGES CXX)

# Find JUCE (adjust components to what you use)
 cmake -DJUCE_DIR="C:/JUCE//" 

if (WIN32)
  enable_language(RC)
endif()

# Collect your sources
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
     "Source/*.cpp" "Source/*.cxx" "Source/*.cc" "Source/*.c"
     "Source/*.mm"  "Source/*.m"
     "Source/*.h"   "Source/*.hpp")

# IMPORTANT: no ICONS arg here — avoids JUCE generating its own icon.rc
juce_add_gui_app(HSP_ProtoFlash
    COMPANY_NAME "Harold Street Pedals"
    PRODUCT_NAME "HSP ProtoFlash"
    BUNDLE_ID    com.haroldstreetpedals.hsp_protoflash
    # WIN32  # uncomment if you want no console window on Windows
)

# Add application sources + explicit resource file path
target_sources(HSP_ProtoFlash PRIVATE
    ${APP_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc
)

if (WIN32)
  set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/resources.rc PROPERTIES LANGUAGE RC)
endif()

target_compile_features(HSP_ProtoFlash PRIVATE cxx_std_17)

target_link_libraries(HSP_ProtoFlash PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

set_target_properties(HSP_ProtoFlash PROPERTIES
    OUTPUT_NAME "HSP_ProtoFlash"
)
