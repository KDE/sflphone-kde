/***************************************************************************
 *   Copyright (C) 2009-2013 by Savoir-Faire Linux                         *
 *   Author : Emmanuel Lepage Valle <emmanuel.lepage@savoirfairelinux.com >*
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 **************************************************************************/

//Parent
#include "bookmarkdock.h"

//Qt
#include <QtGui/QVBoxLayout>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QTreeWidget>
#include <QtGui/QSplitter>
#include <QtGui/QCheckBox>
#include <QStandardItemModel>

//KDE
#include <KLocalizedString>
#include <KIcon>
#include <KLineEdit>

//SFLPhone
#include "klib/kcfg_settings.h"
#include "sflphone.h"
#include "klib/akonadibackend.h"
#include "klib/helperfunctions.h"
#include "klib/bookmarkmodel.h"
#include "lib/historymodel.h"
#include "lib/accountlistmodel.h"
#include "lib/callmodel.h"
#include "categorizedtreeview.h"
#include "../delegates/categorizeddelegate.h"
#include "../delegates/historydelegate.h"

///Constructor
BookmarkDock::BookmarkDock(QWidget* parent) : QDockWidget(parent)
{
   setObjectName("bookmarkDock");
   QWidget* mainWidget     = new QWidget              ( this              );
   setupUi(mainWidget);
   m_pBottomWidget->setHidden(true);
   m_pSortByCBB->setHidden(true);
   m_pSortByL->setHidden(true);
   setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
   setMinimumSize(250,0);
   m_pMostUsedCK           = new QCheckBox            ( this              );

   CategorizedDelegate* delegate = new CategorizedDelegate(m_pView);
   delegate->setChildDelegate(new HistoryDelegate(m_pView));
   m_pView->setDelegate(delegate);
   BookmarkSortFilterProxyModel* m_pProxyModel = new BookmarkSortFilterProxyModel(this);
   m_pProxyModel->setSourceModel(BookmarkModel::instance());
   m_pProxyModel->setFilterRole(Call::Role::Filter);
   m_pProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
   m_pView->setModel(m_pProxyModel);
   expandTree();

   connect(BookmarkModel::instance() ,SIGNAL(layoutChanged()), this , SLOT(expandTree()));
   connect(m_pFilterLE ,SIGNAL(filterStringChanged(QString)), m_pProxyModel , SLOT(setFilterRegExp(QString)));
   connect(m_pFilterLE ,SIGNAL(textChanged(QString)), this , SLOT(expandTree()));

   m_pFilterLE->setPlaceholderText(i18n("Filter"));

   m_pMostUsedCK->setChecked(ConfigurationSkeleton::displayContactCallHistory());

   setWidget(mainWidget);

   m_pMostUsedCK->setText(i18n("Show most called contacts"));

   m_pTopWidget->layout()->addWidget ( m_pMostUsedCK );

   splitter->setStretchFactor(0,7);

   setWindowTitle(i18n("Bookmark"));

   connect(m_pFilterLE                    , SIGNAL(textChanged(QString)),       this , SLOT(filter(QString))             );
   connect(m_pMostUsedCK                  , SIGNAL(toggled(bool)),              this , SLOT(reload())                    );
   connect(AkonadiBackend::instance()  , SIGNAL(collectionChanged()) ,       this , SLOT(reload())                    );
   connect(m_pView                        , SIGNAL(doubleClicked(QModelIndex)), this , SLOT(slotDoubleClick(QModelIndex)));
   reload();
} //BookmarkDock

///Destructor
BookmarkDock::~BookmarkDock()
{
   delete m_pMostUsedCK;
}


/*****************************************************************************
 *                                                                           *
 *                                  Mutator                                  *
 *                                                                           *
 ****************************************************************************/

///Show the most popular items
void BookmarkDock::reload()
{
   ConfigurationSkeleton::setDisplayContactCallHistory(m_pMostUsedCK->isChecked());
   BookmarkModel::instance()->reloadCategories();
} //reload

///Expand the tree according to the user preferences
void BookmarkDock::expandTree()
{
   m_pView->expandToDepth(0);
}

void BookmarkDock::slotDoubleClick(const QModelIndex& index)
{
   QModelIndex idx = (static_cast<const BookmarkSortFilterProxyModel*>(index.model()))->mapToSource(index);
   if (!idx.isValid() || !idx.parent().isValid())
      return;
   if (((CategorizedCompositeNode*)idx.internalPointer())->type() != CategorizedCompositeNode::Type::BOOKMARK)
      return;
   Call* call2 = CallModel::instance()->addDialingCall(idx.model()->data(idx,Call::Role::Number).toString(), AccountListModel::currentAccount());
   call2->setDialNumber  ( idx.model()->data(idx,Call::Role::Number).toString() );
   call2->actionPerformed( Call::Action::ACCEPT   );
}
