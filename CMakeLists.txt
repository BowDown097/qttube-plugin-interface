cmake_minimum_required(VERSION 3.21)

project(qttube-plugin-interface
    VERSION 1.0
    DESCRIPTION "An interface for developing plugins for QtTube."
    HOMEPAGE_URL "https://github.com/BowDown097/qttube-plugin-interface"
    LANGUAGES CXX)

option(ENABLE_WEB_COMPONENTS "Enable usage of WebEngine-based components." OFF)

add_library(QtTube-PluginInterface STATIC)

set_target_properties(QtTube-PluginInterface PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON)

target_include_directories(QtTube-PluginInterface PUBLIC verdigris/src)

target_compile_definitions(QtTube-PluginInterface PUBLIC
    QTTUBE_VERSION_NAME="${PROJECT_VERSION}")

target_sources(QtTube-PluginInterface
    PRIVATE
        qttube-plugin/components/auth/authstore.cpp
        qttube-plugin/components/player/fullscreennotification.cpp
        qttube-plugin/components/settings/settingswindow.cpp
        qttube-plugin/utils/httprequest.cpp
        qttube-plugin/utils/localcache.cpp
    PUBLIC
        FILE_SET HEADERS FILES
            qttube-plugin/plugininterface.h
            qttube-plugin/pluginmetadata.h
            qttube-plugin/components/configstore.h
            qttube-plugin/components/reply.h
            qttube-plugin/components/auth/authroutine.h
            qttube-plugin/components/auth/authstore.h
            qttube-plugin/components/player/fullscreennotification.h
            qttube-plugin/components/player/player.h
            qttube-plugin/components/replytypes/channeldata.h
            qttube-plugin/components/replytypes/initialaccountdata.h
            qttube-plugin/components/replytypes/recommendedcontinuationdata.h
            qttube-plugin/components/replytypes/replytypes.h
            qttube-plugin/components/replytypes/resolveurldata.h
            qttube-plugin/components/replytypes/simpledatatypes.h
            qttube-plugin/components/replytypes/videodata.h
            qttube-plugin/components/settings/playersettings.h
            qttube-plugin/components/settings/settingsstore.h
            qttube-plugin/components/settings/settingswindow.h
            qttube-plugin/objects/badge.h
            qttube-plugin/objects/channel.h
            qttube-plugin/objects/emoji.h
            qttube-plugin/objects/notification.h
            qttube-plugin/objects/shelf.h
            qttube-plugin/objects/video.h
            qttube-plugin/objects/livechat/livechat.h
            qttube-plugin/objects/livechat/livechatbase.h
            qttube-plugin/objects/livechat/livechatreplay.h
            qttube-plugin/objects/livechat/items/giftredemptionmessage.h
            qttube-plugin/objects/livechat/items/paidmessage.h
            qttube-plugin/objects/livechat/items/specialmessage.h
            qttube-plugin/objects/livechat/items/textmessage.h
            qttube-plugin/utils/filesystem.h
            qttube-plugin/utils/httprequest.h
            qttube-plugin/utils/localcache.h
            qttube-plugin/utils/replyutils.h)

if(ENABLE_WEB_COMPONENTS)
    find_package(Qt6 REQUIRED COMPONENTS Network WebEngineWidgets Widgets)
    target_link_libraries(QtTube-PluginInterface PUBLIC Qt::Network Qt::WebEngineWidgets Qt::Widgets)
    target_sources(QtTube-PluginInterface
        PRIVATE
            qttube-plugin/components/auth/webauthroutine.cpp
            qttube-plugin/components/player/webplayer.cpp
        PUBLIC
            FILE_SET HEADERS FILES
                qttube-plugin/components/auth/webauthroutine.h
                qttube-plugin/components/player/webplayer.h)
else()
    find_package(Qt6 REQUIRED COMPONENTS Network Widgets)
    target_link_libraries(QtTube-PluginInterface PUBLIC Qt::Network Qt::Widgets)
endif()

add_library(QtTube::PluginInterface ALIAS QtTube-PluginInterface)

