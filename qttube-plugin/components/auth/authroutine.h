#pragma once
#include <QObject>
#include <QPointer>

namespace QtTubePlugin
{
    class AuthStoreBase;

    class AuthRoutine : public QObject
    {
        Q_OBJECT
    public:
        explicit AuthRoutine(AuthStoreBase* authStore) : m_authStore(authStore) {}
        virtual ~AuthRoutine() = default;
        virtual void start() = 0;
    protected:
        AuthStoreBase* m_authStore;
    signals:
        void success();
    };
}
