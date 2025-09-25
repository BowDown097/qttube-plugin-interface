#pragma once
#include "components/auth/authstore.h"
#include "components/player/player.h"
#include "components/replytypes/replytypes.h"
#include "components/settings/settingsstore.h"
#include "pluginmetadata.h"

#ifdef Q_OS_WIN
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

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
            QFile::exists(QCoreApplication::applicationDirPath() + QDir::separator() + "portable.txt");
        return result;
    }

    static bool isSelfContainedBuild()
    {
        static const bool result = isPortableBuild() ||
            QFile::exists(QCoreApplication::applicationDirPath() + QDir::separator() + "selfcontained.txt");
        return result;
    }
}

using QtTubePluginAuthFunc = QtTubePlugin::AuthStoreBase*(*)();
using QtTubePluginMetadataFunc = QtTubePlugin::PluginMetadata(*)();
using QtTubePluginNewInstanceFunc = QtTubePlugin::PluginInterface*(*)();
using QtTubePluginPlayerFunc = QtTubePlugin::Player*(*)(QWidget*);
using QtTubePluginSettingsFunc = QtTubePlugin::SettingsStore*(*)();
using QtTubePluginTargetVersionFunc = const char*(*)();

#define EXPAND(x) x
#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define DECLARE_QTTUBE_PLUGIN(...) \
EXPAND(GET_MACRO(__VA_ARGS__, \
                 DECLARE_QTTUBE_PLUGIN4, DECLARE_QTTUBE_PLUGIN3, \
                 DECLARE_QTTUBE_PLUGIN2, DECLARE_QTTUBE_PLUGIN1)(__VA_ARGS__))

#define DECLARE_QTTUBE_PLUGIN1(PluginClass) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTubePlugin::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTubePlugin::PluginMetadata metadata() { return { PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_DESCRIPTION, PLUGIN_IMAGE, PLUGIN_AUTHOR, PLUGIN_URL }; } \
    }

#define DECLARE_QTTUBE_PLUGIN2(PluginClass, PlayerClass) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTubePlugin::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTubePlugin::PluginMetadata metadata() { return { PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_DESCRIPTION, PLUGIN_IMAGE, PLUGIN_AUTHOR, PLUGIN_URL }; } \
        DLLEXPORT QtTubePlugin::Player* player(QWidget* parent) { return new PlayerClass(parent); } \
    }

#define DECLARE_QTTUBE_PLUGIN3(PluginClass, PlayerClass, SettingsClass) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTubePlugin::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTubePlugin::PluginMetadata metadata() { return { PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_DESCRIPTION, PLUGIN_IMAGE, PLUGIN_AUTHOR, PLUGIN_URL }; } \
        DLLEXPORT QtTubePlugin::Player* player(QWidget* parent) { return new PlayerClass(parent); } \
        DLLEXPORT QtTubePlugin::Settings* settings() \
        { \
            static std::unique_ptr<SettingsClass> s = QtTubePlugin::SettingsStore::create<SettingsClass>(PLUGIN_NAME, QtTubePlugin::isPortableBuild()); \
            return s.get(); \
        } \
    }

#define DECLARE_QTTUBE_PLUGIN4(PluginClass, PlayerClass, SettingsClass, AuthClass) \
    extern "C" \
    { \
        DLLEXPORT const char* targetVersion() { return QTTUBE_VERSION_NAME; } \
        DLLEXPORT QtTubePlugin::PluginInterface* newInstance() { return new PluginClass; } \
        DLLEXPORT QtTubePlugin::PluginMetadata metadata() { return { PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_DESCRIPTION, PLUGIN_IMAGE, PLUGIN_AUTHOR, PLUGIN_URL }; } \
        DLLEXPORT QtTubePlugin::Player* player(QWidget* parent) { return new PlayerClass(parent); } \
        DLLEXPORT QtTubePlugin::SettingsStore* settings() \
        { \
            static std::unique_ptr<SettingsClass> s = QtTubePlugin::SettingsStore::create<SettingsClass>(PLUGIN_NAME, QtTubePlugin::isPortableBuild()); \
            return s.get(); \
        } \
        DLLEXPORT QtTubePlugin::AuthStoreBase* auth() \
        { \
            static std::unique_ptr<AuthClass> a = QtTubePlugin::AuthStoreBase::create<AuthClass>(PLUGIN_NAME, QtTubePlugin::isPortableBuild()); \
            return a.get(); \
        } \
    }
