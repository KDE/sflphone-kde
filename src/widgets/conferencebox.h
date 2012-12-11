/***************************************************************************
 *   Copyright (C) 2009 by Rafael Fernández López <ereslibre@kde.org>      *
 *                                                                         *
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Library General Public             *
 * License version 2 as published by the Free Software Foundation.         *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Library General Public License for more details.                        *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/
#ifndef CONFERENCEBOX_H
#define CONFERENCEBOX_H

// this code is taken from SystemSettings/icons/CategoryDrawer.{h,cpp}
// Rafael agreet to relicense it under LGPLv2 or LGPLv3, just as we need it,
// see: http://lists.kde.org/?l=kwrite-devel&m=133061943317199&w=2

//KDE
#include <KCategoryDrawer>

//Qt
class QPainter;
class QModelIndex;
class QStyleOption;

///ConferenceBox: Draw a nice box instead of using plain tree
class ConferenceBox : public KCategoryDrawerV2
{
public:
    ConferenceBox();

    virtual void drawCategory(const QModelIndex &index, int sortRole, const QStyleOption &option, QPainter *painter,const QPalette* pal = nullptr) const;
    virtual int categoryHeight(const QModelIndex &index, const QStyleOption &option,const QPalette* pal = nullptr) const;
    void drawBoxBottom(const QModelIndex &index, int sortRole, const QStyleOption &option, QPainter *painter,const QPalette* pal = nullptr) const;
};

#endif
