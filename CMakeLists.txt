cmake_minimum_required(VERSION 3.14.0)

set(vst3sdk_SOURCE_DIR D:/ALL_SDK/VST_SDK/VST3_SDK)
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(CircularGate)
  
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(CircularGate     
    source/version.h
    source/ca_cids.h
    source/ca_processor.h
    source/ca_processor.cpp
    source/ca_controller.h
    source/ca_controller.cpp
    source/ca_entry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
    target_sources(CircularGate
        PRIVATE
            resource/ca_editor.uidesc
    )
    target_link_libraries(CircularGate
        PRIVATE
            vstgui_support
    )
    smtg_add_vst3_resource(CircularGate "resource/ca_editor.uidesc")
endif()
# -------------------

smtg_add_vst3_snapshot(CircularGate resource/2E5F73811CDD56E5A1D88B196E7072F6_snapshot.png)
smtg_add_vst3_snapshot(CircularGate resource/2E5F73811CDD56E5A1D88B196E7072F6_snapshot_2.0x.png)

target_link_libraries(CircularGate
    PRIVATE
        sdk
)

if(SMTG_MAC)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
    smtg_set_bundle(CircularGate
		BUNDLE_IDENTIFIER csse
		INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS
    )
    smtg_set_debug_executable(CircularGate
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(CircularGate PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CircularGate)
    
        smtg_set_debug_executable(CircularGate
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif()
