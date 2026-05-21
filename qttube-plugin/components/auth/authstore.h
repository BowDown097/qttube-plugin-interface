#pragma once
#include "authroutine.h"
#include "qttube-plugin/components/configstore.h"
#include "qttube-plugin/components/replytypes/initialaccountdata.h"

namespace QtTubePlugin
{
    struct AuthUser
    {
        bool active{};
        QString avatar;
        QString handle;
        QString id;
        QString username;

        AuthUser() = default;
        virtual ~AuthUser() = default;

        AuthUser(bool active, const QString& avatar, const QString& id, const QString& username, const QString& handle = {})
            : active(active), avatar(avatar), handle(handle), id(id), username(username) {}
    };

    class AuthStoreBase : public QObject, public ConfigStore
    {
        Q_OBJECT
    public:
        void clear() override;

        // these methods will not unauthenticate for you!
        // it is your responsibility to do so if necessary.
        void drop(AuthUser* login);
        void drop(const std::unique_ptr<AuthUser>& login);

        AuthUser* activeBaseLogin() const;
        const std::vector<std::unique_ptr<AuthUser>>& baseCredentials() const { return m_credentials; }
        bool isEmpty() const;
        qsizetype size() const;

        virtual void restoreFromActive() = 0;
        virtual void startAuthRoutine() = 0;
        virtual void unauthenticate() = 0;
        virtual void update(const InitialAccountData& data) = 0;

        template<typename T> requires std::derived_from<T, AuthStoreBase>
        static std::unique_ptr<T> create(const QString& plugin, bool portable)
        {
            return ConfigStore::create<T>(plugin, "auth", portable);
        }
    protected:
        std::vector<std::unique_ptr<AuthUser>> m_credentials;
    signals:
        void authenticateSuccess();
        void updateFail();
    };

    template<typename UserType, typename RoutineType>
        requires std::derived_from<UserType, AuthUser> &&
                 std::derived_from<RoutineType, AuthRoutine> &&
                 std::constructible_from<RoutineType, AuthStoreBase*>
    class AuthStore : public AuthStoreBase
    {
    public:
        virtual UserType createUser(const InitialAccountData& data, const RoutineType* routine) = 0;

        UserType* activeLogin() const
        {
            auto it = std::ranges::find_if(m_credentials, &UserType::active);
            return it != m_credentials.end() ? static_cast<UserType*>(it->get()) : nullptr;
        }

        void append(UserType&& user)
        {
            m_credentials.push_back(std::make_unique<UserType>(std::forward<UserType>(user)));
        }

        QList<UserType*> credentials() const
        {
            QList<UserType*> out;
            out.reserve(m_credentials.size());
            for (const std::unique_ptr<AuthUser>& user : m_credentials)
                out.append(static_cast<UserType*>(user.get()));
            return out;
        }

        void startAuthRoutine() override
        {
            m_activeRoutine.reset(new RoutineType(this));
            connect(m_activeRoutine.data(), &RoutineType::success, this, &AuthStoreBase::authenticateSuccess);
            m_activeRoutine->start();
        }

        void update(const InitialAccountData& data) override
        {
            if (data.channelId.isEmpty())
            {
                m_activeRoutine.reset();
                emit updateFail();
                return;
            }

            if (AuthUser* user = activeBaseLogin())
            {
                if (user->id != data.channelId)
                {
                    user->active = false;
                    auto match = std::ranges::find(m_credentials, data.channelId, &AuthUser::id);
                    if (match != m_credentials.end())
                    {
                        AuthUser* matchUser = match->get();
                        matchUser->active = true;
                        matchUser->avatar = data.avatarUrl;
                        matchUser->handle = data.handle;
                        matchUser->username = data.displayName;
                    }
                }
                else
                {
                    user->avatar = data.avatarUrl;
                    user->handle = data.handle;
                    user->username = data.displayName;
                }
            }
            else
            {
                append(createUser(data, m_activeRoutine.data()));
            }

            m_activeRoutine.reset();
            if (!isEmpty())
                save();
        }
    private:
        QScopedPointer<RoutineType, QScopedPointerDeleteLater> m_activeRoutine;
    };
}
