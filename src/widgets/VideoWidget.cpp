/************************************************************************************
 *   Copyright (C) 2012 by Savoir-Faire Linux                                       *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>         *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/
#include "VideoWidget.h"
#include <KDebug>

///Constructor
VideoWidget::VideoWidget(QWidget* parent) : QWidget(parent),m_Image(nullptr) {
   setMinimumSize(200,200);
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   connect(VideoModel::getInstance(),SIGNAL(frameUpdated()),this,SLOT(updateFrame()));
   connect(VideoModel::getInstance(),SIGNAL(videoStopped()),this,SLOT(stop()));
}

///Repaint the widget
void VideoWidget::update() {
   kDebug() << "Painting" << this;
   QPainter painter(this);
   if (m_Image && VideoModel::getInstance()->isRendering())
      painter.drawImage(QRect(0,0,width(),height()),*(m_Image));
   painter.end();
}

///Called when the widget need repainting
void VideoWidget::paintEvent(QPaintEvent* event)
{
   Q_UNUSED(event)
   //if (VideoModel::getInstance()->isPreviewing()) {
   update();
   //}
}

///Called when a new frame is ready
void VideoWidget::updateFrame()
{
   QSize size(VideoModel::getInstance()->getActiveResolution().width, VideoModel::getInstance()->getActiveResolution().height);
   if (size != minimumSize())
      setMinimumSize(size);
   if (m_Image)
      delete m_Image;
   m_Image = new QImage((uchar*)VideoModel::getInstance()->getCurrentFrame().data() , size.width(), size.height(), QImage::Format_ARGB32 );
   //This is the right way to do it, but it does not work
//    if (!m_Image || (m_Image && m_Image->size() != size))
//       m_Image = new QImage((uchar*)VideoModel::getInstance()->rawData() , size.width(), size.height(), QImage::Format_ARGB32 );
//    if (!m_Image->loadFromData(VideoModel::getInstance()->getCurrentFrame())) {
//       qDebug() << "Loading image failed";
//    }
   repaint();
}

///Prevent the painter to try to paint an invalid framebuffer
void VideoWidget::stop()
{
   if (m_Image) {
      delete m_Image;
      m_Image = nullptr;
   }
}