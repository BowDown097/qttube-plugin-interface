#pragma once
#include "qttube-plugin/utils/filesystem.h"
#include <QCoreApplication>
#include <QSettings>
#include <QStandardPaths>

namespace QtTubePlugin
{
    class ConfigStore
    {
    public:
        virtual ~ConfigStore() = default;
        virtual void clear() { QSettings(configPath(), QSettings::IniFormat).clear(); }
        virtual void init() = 0;
        virtual void save() = 0;

        const QString& configPath() const { return m_configPath; }

        template<typename T> requires std::derived_from<T, ConfigStore>
        static std::unique_ptr<T> create(const QString& plugin, const QString& key, bool portable)
        {
            auto inst = std::make_unique<T>();
            inst->m_configPath = resolveConfigPath(plugin, key, portable);
            return inst;
        }

        static QString resolveConfigPath(const QString& plugin, const QString& key, bool portable)
        {
            const QString appConfigLocation = portable
                ? FS::joinPaths(QCoreApplication::applicationDirPath(), "config")
                : QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
            return FS::joinPaths(appConfigLocation, plugin.toLower().replace(' ', '-'), key + ".ini");
        }
    protected:
        void setConfigPath(const QString& configPath) { m_configPath = configPath; }
        void setConfigPath(QString&& configPath) { m_configPath = std::move(configPath); }

        template<typename T> requires std::constructible_from<QVariant, T>
        void readIntoList(QSettings& settings, QList<T>& list, const QString& prefix, const QString& key)
        {
            list.clear();
            int sz = settings.beginReadArray(prefix);
            for (int i = 0; i < sz; ++i)
            {
                settings.setArrayIndex(i);
                list.append(settings.value(key).value<T>());
            }
            settings.endArray();
        }

        template<typename T> requires std::constructible_from<QVariant, T>
        void writeList(QSettings& settings, const QList<T>& list, const QString& prefix, const QString& key)
        {
            settings.beginWriteArray(prefix);
            for (int i = 0; i < list.size(); ++i)
            {
                settings.setArrayIndex(i);
                settings.setValue(key, list.at(i));
            }
            settings.endArray();
        }
    private:
        QString m_configPath;
    };
}
