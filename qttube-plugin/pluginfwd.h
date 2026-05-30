#pragma once
#include <concepts>

class QWidget;

namespace QtTubePlugin
{
struct AuthUser;
struct Badge;
struct Channel;
struct ChannelData;
struct ChannelHeader;
struct ChannelTabData;
struct Emoji;
struct GiftRedemptionMessage;
struct InitialAccountData;
struct InitialLiveChatData;
struct LiveChat;
struct LiveChatBase;
struct LiveChatReplay;
struct LiveChatViewOption;
struct Notification;
struct NotificationBell;
struct NotificationState;
struct PaidMessage;
struct PluginInterface;
struct PluginMetadata;
struct RecommendedContinuationData;
struct ResolveUrlData;
struct SearchCookie;
struct SettingsStore;
struct ShelfBase;
struct SpecialMessage;
struct SubscribeButton;
struct TextMessage;
struct Video;
struct VideoData;

class AuthRoutine;
class AuthStoreBase;
class ConfigStore;
class FullScreenNotification;
class FullScreenWindow;
class Exception;
class Player;
class PlayerSettings;
class SettingsWindow;
class WebAuthRoutine;
class WebChannelInterface;
class WebPlayer;

template<typename UserType, typename RoutineType>
    requires std::derived_from<UserType, AuthUser> &&
             std::derived_from<RoutineType, AuthRoutine> &&
             std::constructible_from<RoutineType, AuthStoreBase*>
class AuthStore;

template<typename> class Reply;
template<typename...> struct Shelf;
}

using QtTubePluginAuthFunc = QtTubePlugin::AuthStoreBase*(*)();
using QtTubePluginMetadataFunc = QtTubePlugin::PluginMetadata(*)();
using QtTubePluginNewInstanceFunc = QtTubePlugin::PluginInterface*(*)();
using QtTubePluginPlayerFunc = QtTubePlugin::Player*(*)(QtTubePlugin::PlayerSettings*, QWidget*);
using QtTubePluginSettingsFunc = QtTubePlugin::SettingsStore*(*)();
using QtTubePluginTargetVersionFunc = const char*(*)();