function(create_qttube_plugin target)
    # check if given target is actually a target
    if(NOT TARGET ${target})
        message(FATAL_ERROR "'${target}' is not a target.")
    endif()

    # check for presence of metadata file
    set(metadata_file "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json")
    if(NOT EXISTS "${metadata_file}")
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

    # helper function for required JSON fields
    function(json_get_required out json key)
        string(JSON ${out} ERROR_VARIABLE err GET "${json}" ${key})
        if(err)
            message(FATAL_ERROR "Missing required field in metadata.json: ${key}")
        endif()
        set(${out} "${${out}}" PARENT_SCOPE)
    endfunction()

    # helper function for optional JSON fields
    function(json_get_optional out json key)
        string(JSON ${out} ERROR_VARIABLE err GET "${json}" ${key})
        if(err)
            message(WARNING "Missing optional field in metadata.json: ${key} - it is highly recommended to provide this")
            set(${out} "" PARENT_SCOPE)
        else()
            set(${out} "${${out}}" PARENT_SCOPE)
        endif()
    endfunction()

    # attempt to extract metadata from metadata.json
    file(READ "${metadata_file}" metadata_json)

    json_get_required(plugin_name        "${metadata_json}" name)
    json_get_required(plugin_version     "${metadata_json}" version)
    json_get_required(plugin_description "${metadata_json}" description)

    json_get_optional(plugin_url         "${metadata_json}" url)
    json_get_optional(plugin_image       "${metadata_json}" image)
    json_get_optional(plugin_author      "${metadata_json}" author)

    # save metadata as compile definitions
    target_compile_definitions(${target} PRIVATE
        PLUGIN_NAME="${plugin_name}"
        PLUGIN_VERSION="${plugin_version}"
        PLUGIN_DESCRIPTION="${plugin_description}"
        PLUGIN_URL="${plugin_url}"
        PLUGIN_IMAGE="${plugin_image}"
        PLUGIN_AUTHOR="${plugin_author}")
endfunction()
