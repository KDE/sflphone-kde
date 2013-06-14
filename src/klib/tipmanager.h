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
#ifndef TIPMANAGER_H
#define TIPMANAGER_H

//Qt
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtGui/QPalette>
#include <QtGui/QTreeView>
#include <QtGui/QImage>
class QTimer;

//SFLPhone
#include "../lib/typedefs.h"
#include "tip.h"
#include "tipanimationwrapper.h"
class TipManager;

///This class create a background brush for a QWidget with a tip window
class LIB_EXPORT TipManager : public QObject
{
   Q_OBJECT
   friend class ResizeEventFilter;
public:
   //Constructor
   explicit TipManager(QTreeView* parent);
   virtual ~TipManager();

   //Mutator
   void hideTip(Tip* tip);

   //Getter
   QImage getImage();
   Tip* currentTip() {return m_pCurrentTip;}

   //Setters
   void setTopMargin(int margin);
   void setBottomMargin(int margin);
   void setCurrentTip(Tip* tip);

protected:
   bool eventFilter(QObject *obj, QEvent *event);

private:
   //Methods
   void reload();
   void setCurrentTip_private(Tip* tip);

   //Attributes
   QPalette                m_OriginalPalette ;
   QTreeView*              m_pParent         ;
   int                     m_TopMargin       ;
   int                     m_BottomMargin    ;
   QImage                  m_CurrentImage    ;
   Tip*                    m_pCurrentTip     ;
   TipAnimationWrapper     m_pAnim           ;
   FrameDescription        m_CurrentFrame    ;
   QList<Tip*>             m_lTipQueue       ;
   QList<Tip*>             m_lHidingTipQueue ;
   QTimer*                 m_pTimer          ;

private Q_SLOTS:
   void animationStep(FrameDescription desc);
   void changeSize(bool ignoreAnim = false);
   void animationEnded();
   void timeout();

Q_SIGNALS:
   void sizeChanged(QRect newRect,bool ignoreAnim);
   void currentTipChanged(Tip*);

};
Q_DECLARE_METATYPE(TipManager*)

#endif
