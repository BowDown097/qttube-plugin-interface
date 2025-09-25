#pragma once
#include <QDir>

namespace FS
{
    template<typename T>
    concept QStringAppendable = requires(QString& a, T b) { a.append(b); QAnyStringView(b); };

    QString joinPaths(QString path, QStringAppendable auto&&... paths)
    {
        ([&] {
            if (!QAnyStringView(paths).isEmpty())
            {
                if (!path.isEmpty())
                    path.append(QDir::separator());
                path.append(std::forward<decltype(paths)>(paths));
            }
        }(), ...);
        return QDir::cleanPath(path);
    }
}
