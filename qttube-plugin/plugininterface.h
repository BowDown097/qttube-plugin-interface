#pragma once
#include "components/auth/authstore.h"
#include "components/player/player.h"
#include "components/replytypes/replytypes.h"
#include "components/settings/settingsstore.h"
#include "pluginmetadata.h"

namespace QtTubePlugin
{
    struct PluginInterface
    {
        virtual ~PluginInterface() = default;

        virtual AccountReply* getActiveAccount() { return nullptr; }

        virtual ChannelReply* getChannel(
            const QString& channelId,
            std::any tabData,
            std::any continuationData) { return nullptr; }

        virtual BrowseReply* getHistory(const QString& query, std::any continuationData) { return nullptr; }
        virtual BrowseReply* getHome(std::any continuationData) { return nullptr; }
        virtual NotificationsReply* getNotifications(std::any continuationData) { return nullptr; }
        virtual BrowseReply* getSearch(
            const QString& query,
            const QList<std::pair<QString, int>>& activeFilters, // mapped as category -> index
            std::any continuationData) { return nullptr; }
        virtual BrowseReply* getSubFeed(std::any continuationData) { return nullptr; }
        virtual BrowseReply* getTrending(std::any continuationData) { return nullptr; }

        virtual RecommendedContinuationReply* continueRecommended(
            const QString& videoId,
            std::any continuationData) { return nullptr; }

        virtual VideoReply* getVideo(const QString& videoId) { return nullptr; }

        virtual LiveChatReply* getLiveChat(std::any data) { return nullptr; }
        virtual LiveChatReplayReply* getLiveChatReplay(std::any data, qint64 videoOffsetMs) { return nullptr; }
        virtual Reply<void>* sendLiveChatMessage(const QString& text) { return nullptr; }

        virtual Reply<void>* rate(
            const QString& videoId,
            bool like,
            bool removing,
            std::any data) { return nullptr; }

        virtual Reply<void>* setNotificationPreference(std::any data) { return nullptr; }
        virtual Reply<void>* subscribe(std::any data) { return nullptr; }
        virtual Reply<void>* unsubscribe(std::any data) { return nullptr; }

        virtual ResolveUrlReply* resolveUrlOrID(const QString& in) { return nullptr; }

        virtual void init() = 0;

        // mapped as category -> filters
        virtual const QList<std::pair<QString, QStringList>> searchFilters() const { return {}; }
    };

    static bool isPortableBuild()
    {
        static const bool result =
            QFile::exists(FS::joinPaths(QCoreApplication::applicationDirPath(), "portable.txt"));
        return result;
    }

    static bool isSelfContainedBuild()
    {
        static const bool result = isPortableBuild() ||
            QFile::exists(FS::joinPaths(QCoreApplication::applicationDirPath(), "selfcontained.txt"));
        return result;
    }
}

using QtTubePluginAuthFunc = QtTubePlugin::AuthStoreBase*(*)();
using QtTubePluginMetadataFunc = QtTubePlugin::PluginMetadata(*)();
using QtTubePluginNewInstanceFunc = QtTubePlugin::PluginInterface*(*)();
using QtTubePluginPlayerFunc = QtTubePlugin::Player*(*)(QtTubePlugin::PlayerSettings*, QWidget*);
using QtTubePluginSettingsFunc = QtTubePlugin::SettingsStore*(*)();
using QtTubePluginTargetVersionFunc = const char*(*)();

#define QTTUBE_EXPAND(x) x
#define QTTUBE_GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define DECLARE_QTTUBE_PLUGIN(...) \
QTTUBE_EXPAND(QTTUBE_GET_MACRO(__VA_ARGS__, \
    DECLARE_QTTUBE_PLUGIN4, DECLARE_QTTUBE_PLUGIN3, \
    DECLARE_QTTUBE_PLUGIN2, DECLARE_QTTUBE_PLUGIN1)(__VA_ARGS__))


#define QTTUBE_TARGET_VERSION_FUNC \
    Q_DECL_EXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; }

#define QTTUBE_METADATA_FUNC \
    Q_DECL_EXPORT QtTubePlugin::PluginMetadata metadata() \
    { \
        return { \
            .name = PLUGIN_NAME, \
            .version = PLUGIN_VERSION, \
            .description = PLUGIN_DESCRIPTION, \
            .image = PLUGIN_IMAGE, \
            .author = PLUGIN_AUTHOR, \
            .url = PLUGIN_URL, \
            .channelUrlTemplate = PLUGIN_CHANNEL_URL_TEMPLATE, \
            .videoUrlTemplate = PLUGIN_VIDEO_URL_TEMPLATE \
        }; \
    }

#define QTTUBE_NEW_INSTANCE_FUNC(PluginClass) \
    QtTubePlugin::PluginInterface* newInstance() { return new PluginClass; }

#define QTTUBE_PLAYER_FUNC(PlayerClass) \
    Q_DECL_EXPORT QtTubePlugin::Player* player(QtTubePlugin::PlayerSettings* settings, QWidget* parent) \
    { return new PlayerClass(settings, parent); }

#define QTTUBE_CREATE_STORE(StoreClass, UserClass) \
    static std::unique_ptr<UserClass> p = QtTubePlugin::StoreClass::create<UserClass>(\
        PLUGIN_NAME, QtTubePlugin::isPortableBuild())

#define QTTUBE_SETTINGS_FUNC(SettingsClass) \
    Q_DECL_EXPORT QtTubePlugin::SettingsStore* settings() { QTTUBE_CREATE_STORE(SettingsStore, SettingsClass); return p.get(); }

#define QTTUBE_AUTH_FUNC(AuthClass) \
    Q_DECL_EXPORT QtTubePlugin::AuthStoreBase* auth() { QTTUBE_CREATE_STORE(AuthStoreBase, AuthClass); return p.get(); }

#define DECLARE_QTTUBE_PLUGIN1(PluginClass) \
    extern "C" \
    { \
        QTTUBE_TARGET_VERSION_FUNC \
        QTTUBE_METADATA_FUNC \
        QTTUBE_NEW_INSTANCE_FUNC(PluginClass) \
    }

#define DECLARE_QTTUBE_PLUGIN2(PluginClass, PlayerClass) \
    extern "C" \
    { \
        QTTUBE_TARGET_VERSION_FUNC \
        QTTUBE_METADATA_FUNC \
        QTTUBE_NEW_INSTANCE_FUNC(PluginClass) \
        QTTUBE_PLAYER_FUNC(PlayerClass) \
    }

#define DECLARE_QTTUBE_PLUGIN3(PluginClass, PlayerClass, SettingsClass) \
    extern "C" \
    { \
        QTTUBE_TARGET_VERSION_FUNC \
        QTTUBE_METADATA_FUNC \
        QTTUBE_NEW_INSTANCE_FUNC(PluginClass) \
        QTTUBE_PLAYER_FUNC(PlayerClass) \
        QTTUBE_SETTINGS_FUNC(SettingsClass) \
    }

#define DECLARE_QTTUBE_PLUGIN4(PluginClass, PlayerClass, SettingsClass, AuthClass) \
    extern "C" \
    { \
        QTTUBE_TARGET_VERSION_FUNC \
        QTTUBE_METADATA_FUNC \
        QTTUBE_NEW_INSTANCE_FUNC(PluginClass) \
        QTTUBE_PLAYER_FUNC(PlayerClass) \
        QTTUBE_SETTINGS_FUNC(SettingsClass) \
        QTTUBE_AUTH_FUNC(AuthClass) \
    }
