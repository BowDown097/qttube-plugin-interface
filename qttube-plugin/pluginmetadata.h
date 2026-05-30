#pragma once
#include <QString>

namespace QtTubePlugin
{
    struct PluginMetadata
    {
        QString name;
        QString version;
        QString description;
        QString image;
        QString author;
        QString url;
        QString channelUrlTemplate;
        QString videoUrlTemplate;
    };
}
