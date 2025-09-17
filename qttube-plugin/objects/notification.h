#pragma once
#include <QString>

namespace QtTubePlugin
{
    struct Notification
    {
        enum class TargetType { Channel, Video };

        QString body;
        QString channelAvatarUrl;
        QString channelId;
        QString channelUrlPrefix;
        QString notificationId;
        QString sentTimeText;
        QString targetId;
        TargetType targetType;
        QString targetUrlPrefix;
        QString thumbnailUrl;
    };
}
