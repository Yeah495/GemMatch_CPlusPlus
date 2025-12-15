#pragma once
/*为了避免每次创建宝石都重新读取硬盘图片，使用单例模式缓存图片资源。000
*/



#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QPixmap>
#include <QMap>
#include <QString>
#include "Config.h"

class ResourceLoader {
public:
    static ResourceLoader& instance() {
        static ResourceLoader instance;
        return instance;
    }

    // 根据宝石类型获取对应图片
    QPixmap getGemPixmap(GemType type) {
        if (!m_gemPixmaps.contains(type)) {
            // 假设资源路径格式为 ":/assets/gem_1.png"
            // 实际项目中需要确保 .qrc 资源文件已配置
            QString path = QString(":/assets/gem_%1.png").arg(static_cast<int>(type));
            m_gemPixmaps[type] = QPixmap(path);
        }
        return m_gemPixmaps[type];
    }

    QPixmap getBackground() {
        if (m_bg.isNull()) m_bg.load(":/assets/background.png");
        return m_bg;
    }

private:
    ResourceLoader() {}
    QMap<GemType, QPixmap> m_gemPixmaps;
    QPixmap m_bg;
};

#endif // RESOURCELOADER_H