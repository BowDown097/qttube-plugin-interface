#include "authstore.h"

namespace QtTubePlugin
{
    AuthUser* AuthStoreBase::activeBaseLogin() const
    {
        auto it = std::ranges::find_if(m_credentials, &AuthUser::active);
        return it != m_credentials.end() ? it->get() : nullptr;
    }

    void AuthStoreBase::clear()
    {
        ConfigStore::clear();
        m_credentials.clear();
        unauthenticate();
    }

    void AuthStoreBase::drop(AuthUser* login)
    {
        auto it = std::ranges::find(m_credentials, login, &std::unique_ptr<AuthUser>::get);
        QSettings(configPath(), QSettings::IniFormat).remove(it->get()->id);
        m_credentials.erase(it);
    }

    void AuthStoreBase::drop(const std::unique_ptr<AuthUser>& login)
    {
        auto it = std::ranges::find(m_credentials, login);
        QSettings(configPath(), QSettings::IniFormat).remove(it->get()->id);
        m_credentials.erase(it);
    }

    bool AuthStoreBase::isEmpty() const
    {
        return m_credentials.empty();
    }

    qsizetype AuthStoreBase::size() const
    {
        return m_credentials.size();
    }
}
