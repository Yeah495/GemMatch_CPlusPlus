#pragma once


#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QPixmap>
#include <QMap>
#include <QString>
#include "Config.h"
#include <QFile>
#include <QCoreApplication>

class ResourceLoader {
public:
    static ResourceLoader& instance() {
        static ResourceLoader instance;
        return instance;
    }

    QPixmap getGemPixmap(GemType type) {
        if (!m_gemPixmaps.contains(type)) {
            QPixmap pix;
            int idx = static_cast<int>(type);
            if (idx <= 0) {
                m_gemPixmaps[type] = pix;
                return pix;
            }

            
            QString resPath = QString(":/assets/images/gem_%1.png").arg(idx);
            if (QFile::exists(resPath)) pix.load(resPath);

            
            if (pix.isNull()) {
                QString rel = QString("assets/images/gem_%1.png").arg(idx);
                if (QFile::exists(rel)) pix.load(rel);
                else {
                    QString appDir = QCoreApplication::applicationDirPath();
                    QString up = appDir + "/../assets/images/gem_" + QString::number(idx) + ".png";
                    if (QFile::exists(up)) pix.load(up);
                }
            }

            m_gemPixmaps[type] = pix;
        }
        return m_gemPixmaps[type];
    }

    QPixmap getBackground() {
        if (m_bg.isNull()) {
            
            QString resBg = ":/assets/images/bg_login.jpg";
            if (QFile::exists(resBg)) m_bg.load(resBg);
            else {
                
                QString rel = QString("assets/images/bg_login.jpg");
                if (QFile::exists(rel)) m_bg.load(rel);
                else {
                    QString appDir = QCoreApplication::applicationDirPath();
                    QString up = appDir + "/../assets/images/bg_login.jpg";
                    if (QFile::exists(up)) m_bg.load(up);
                }
            }
        }
        return m_bg;
    }

private:
    ResourceLoader() {}
    QMap<GemType, QPixmap> m_gemPixmaps;
    QPixmap m_bg;
};

#endif