#pragma once
#include <QString>

namespace QtTubePlugin
{
    struct Badge
    {
        struct ColorPalette
        {
            QString background = "#777";
            QString foreground = "#ddd";
            QString hoveredBackground = "#4aa1df";
            QString hoveredForeground = "#ddd";
        };

        ColorPalette colorPalette;
        QString label;
        QString tooltip;
    };
}
