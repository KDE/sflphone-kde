/****************************************************************************
 *   Copyright (C) 2012-2013 by Savoir-Faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#ifndef CONTACTDELEGATE_H
#define CONTACTDELEGATE_H

#include <QtGui/QStyledItemDelegate>
#include <QtGui/QProxyStyle>
#include <QtCore/QDebug>

class DelegateDropOverlay;

class ContactDelegate : public QStyledItemDelegate
{
public:
   explicit ContactDelegate(QObject* parent = nullptr);

   QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

   virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
private:
   DelegateDropOverlay* m_pDelegatedropoverlay;
   static QHash<QString, QPixmap> m_hIcons;
};

// class ContactDelegateStyle : public QProxyStyle
// {
// public:
//    ContactDelegateStyle(QStyle *baseStyle = 0) : QProxyStyle(baseStyle) {}
// 
//    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
// private:
//    QModelIndex m_CurrentIndex;
// };

#endif
