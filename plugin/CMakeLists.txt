cmake_minimum_required(VERSION 3.22)
project(AudioPlugin VERSION 0.1.0)

juce_add_plugin(AudioPlugin
    COMPANY_NAME "BobsPluginBargainBin"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE BPBB
    FORMATS VST3
    PRODUCT_NAME "AngryTodd"
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/AngryTodd)

set(SOURCE_FILES
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

set(HEADER_FILES
    ${INCLUDE_DIR}/PluginProcessor.h
    ${INCLUDE_DIR}/PluginEditor.h
)

target_sources(${PROJECT_NAME}
    PUBLIC
        ${SOURCE_FILES}
        ${HEADER_FILES}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
