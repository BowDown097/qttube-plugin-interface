function(create_qttube_plugin target)
    # check if given target is actually a target
    if(NOT TARGET ${target})
        message(FATAL_ERROR "'${target}' is not a target.")
    endif()

    # check for presence of metadata file
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")
        message(FATAL_ERROR "metadata.json not found in root project directory.")
    endif()

    # set useful Qt properties and apply the below routine to builds in addition to installs
    set_target_properties(${target} PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
        BUILD_WITH_INSTALL_RPATH ON)

    # provides functionality for linking to libraries from ../plugin-libs on Unix OSes
    # on Windows, this is done in a makeshift way in QtTube itself, since this functionality does not exist
    if(UNIX AND NOT APPLE)
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN/../plugin-libs")
    elseif(APPLE)
        set_target_properties(${target} PROPERTIES INSTALL_RPATH "@loader_path/../plugin-libs")
    endif()

    # attempt to extract data from metadata.json
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json" metadata_json)

    ## get name - required
    string(JSON plugin_name GET "${metadata_json}" name)

    ## get version (with fallback to PROJECT_VERSION if possible) - required
    if(PROJECT_VERSION)
        string(JSON plugin_version ERROR_VARIABLE json_error GET "${metadata_json}" version)
        if(json_error)
            message(WARNING "Plugin metadata provided no version. Falling back to project version (${PROJECT_VERSION})")
            set(plugin_version "${PROJECT_VERSION}")
        endif()
    else()
        string(JSON plugin_version GET "${metadata_json}" version)
    endif()

    ## get description (with fallback to PROJECT_DESCRIPTION if possible) - required
    if(PROJECT_DESCRIPTION)
        string(JSON plugin_description ERROR_VARIABLE json_error GET "${metadata_json}" description)
        if(json_error)
            message(WARNING "Plugin metadata provided no description. Falling back to project description (${PROJECT_DESCRIPTION})")
            set(plugin_description "${PROJECT_DESCRIPTION}")
        endif()
    else()
        string(JSON plugin_description GET "${metadata_json}" description)
    endif()

    ## get url (with fallback to PROJECT_HOMEPAGE_URL if possible) - optional
    string(JSON plugin_url ERROR_VARIABLE json_error GET "${metadata_json}" url)
    if(json_error)
        if(PROJECT_HOMEPAGE_URL)
            message(WARNING "Plugin metadata provided no URL. Falling back to project homepage URL (${PROJECT_HOMEPAGE_URL})")
            set(plugin_url "${PROJECT_HOMEPAGE_URL}")
        else()
            set(plugin_url "")
        endif()
    endif()

    ## get image - optional, but with nag
    string(JSON plugin_image ERROR_VARIABLE json_error GET "${metadata_json}" image)
    if(json_error)
        message(WARNING "Plugin metadata provided no image. It is recommended to provide one.")
        set(plugin_image "")
    endif()

    ## get author - optional, but with nag
    string(JSON plugin_author ERROR_VARIABLE json_error GET "${metadata_json}" author)
    if(json_error)
        message(WARNING "Plugin metadata provided no author. It is recommended to provide one.")
        set(plugin_author "")
    endif()

    # save extracted data to compile definitions
    target_compile_definitions(${target} PRIVATE
        PLUGIN_NAME="${plugin_name}"
        PLUGIN_VERSION="${plugin_version}"
        PLUGIN_DESCRIPTION="${plugin_description}"
        PLUGIN_URL="${plugin_url}"
        PLUGIN_IMAGE="${plugin_image}"
        PLUGIN_AUTHOR="${plugin_author}")
endfunction()
