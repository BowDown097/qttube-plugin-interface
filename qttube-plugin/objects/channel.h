#pragma once
#include "qttube-plugin/objects/badge.h"
#include <any>
#include <QList>

namespace QtTubePlugin
{
    struct NotificationState
    {
        enum class Representation { All, None, Neutral };

        std::any data;
        QString name;
        Representation representation;
    };

    struct NotificationBell
    {
        qsizetype activeStateIndex = -1;
        qsizetype defaultEnabledStateIndex = -1;
        QList<NotificationState> states;
    };

    struct SubscribeButton
    {
        struct ColorPalette
        {
            QString subscribeBackground = "#e62117";
            QString subscribeBorder = "transparent";
            QString subscribeForeground = "#fefefe";

            QString subscribeDisabledBackground = "#f28f8a";
            QString subscribeDisabledBorder = "transparent";
            QString subscribeDisabledForeground = "#f9f9f9";

            QString subscribeHoveredBackground = "#d90a17";
            QString subscribeHoveredBorder = "transparent";
            QString subscribeHoveredForeground = "#fefefe";

            QString subscribedBackground;
            QString subscribedBorder = "#555";
            QString subscribedForeground;

            QString unsubscribeBackground;
            QString unsubscribeBorder = "#555";
            QString unsubscribeForeground;
        };

        struct Localization
        {
            QString subscribeText;
            QString subscribedText;
            QString unsubscribeDialogText;
            QString unsubscribeText;
        };

        ColorPalette colorPalette;
        QString countText;
        bool enabled = true;
        Localization localization;
        NotificationBell notificationBell;
        bool subscribed{};
        std::any subscribeData;
        std::any unsubscribeData;
    };

    struct Channel
    {
        QString channelAvatarUrl;
        QList<Badge> channelBadges;
        QString channelId;
        QString channelName;
        QString channelUrlPrefix;
        QString description;
        QString metadataText;
        SubscribeButton subscribeButton;
    };
}
