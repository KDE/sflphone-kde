/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
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
#include "itembackendmodel.h"

#include "commonbackendmanagerinterface.h"
#include "visitors/itemmodelstateserializationvisitor.h"
#include "abstractitembackendmodelextension.h"

class CommonItemBackendModelPrivate : public QObject
{
   Q_OBJECT
public:
   CommonItemBackendModelPrivate(CommonItemBackendModel* parent);

   /*
    * This is not very efficient, it doesn't really have to be given the low
    * volume. If it ever have to scale, a better mapToSource using persistent
    * index have to be implemented.
    */
   struct ProxyItem {
      ProxyItem() : parent(nullptr),col(1),row(0),backend(nullptr){}
      int row;
      int col;
      AbstractContactBackend* backend;
      ProxyItem* parent;
      QVector<ProxyItem*> m_Children;
   };
   QHash<AbstractContactBackend*,ProxyItem*> m_hBackendsNodes;
   QVector<ProxyItem*> m_lTopLevelBackends;
   QVector<AbstractItemBackendModelExtension*> m_lExtensions;

private:
   CommonItemBackendModel* q_ptr;

private Q_SLOTS:
   void slotUpdate();
   void slotExtensionDataChanged(const QModelIndex& idx);
};

CommonItemBackendModelPrivate::CommonItemBackendModelPrivate(CommonItemBackendModel* parent) : QObject(parent),q_ptr(parent)
{}

CommonItemBackendModel::CommonItemBackendModel(QObject* parent) : QAbstractTableModel(parent), d_ptr(new CommonItemBackendModelPrivate(this))
{
   connect(ContactModel::instance(),SIGNAL(newBackendAdded(AbstractContactBackend*)),d_ptr.data(),SLOT(slotUpdate()));
   load();
}

CommonItemBackendModel::~CommonItemBackendModel()
{
   while (d_ptr->m_lTopLevelBackends.size()) {
      CommonItemBackendModelPrivate::ProxyItem* item = d_ptr->m_lTopLevelBackends[0];
      d_ptr->m_lTopLevelBackends.remove(0);
      while (item->m_Children.size()) {
         //FIXME I don't think it can currently happen, but there may be
         //more than 2 levels.
         CommonItemBackendModelPrivate::ProxyItem* item2 = item->m_Children[0];
         item->m_Children.remove(0);
         delete item2;
      }
      delete item;
   }
}

QVariant CommonItemBackendModel::data (const QModelIndex& idx, int role) const
{
   if (idx.isValid()) {
      CommonItemBackendModelPrivate::ProxyItem* item = static_cast<CommonItemBackendModelPrivate::ProxyItem*>(idx.internalPointer());

      if (idx.column() > 0)
         return d_ptr->m_lExtensions[idx.column()-1]->data(item->backend,idx,role);

      switch(role) {
         case Qt::DisplayRole:
            return item->backend->name();
            break;
         case Qt::DecorationRole:
            return item->backend->icon();
            break;
//          case Qt::CheckStateRole:
//             return item->backend->isEnabled()?Qt::Checked:Qt::Unchecked;
         case Qt::CheckStateRole: {
            if (ItemModelStateSerializationVisitor::instance())
               return ItemModelStateSerializationVisitor::instance()->isChecked(item->backend)?Qt::Checked:Qt::Unchecked;
         }
      };
   }
   //else {
//       CommonItemBackendModelPrivate::ProxyItem* item = static_cast<CommonItemBackendModelPrivate::ProxyItem*>(idx.internalPointer());
//       return item->model->data(item->model->index(item->row,item->col));
   //}
   return QVariant();
}

int CommonItemBackendModel::rowCount (const QModelIndex& parent) const
{
   if (!parent.isValid()) {
      static bool init = false; //FIXME this doesn't allow dynamic backends
      static int result = 0;
      if (!init) {
         for(int i=0;i<ContactModel::instance()->backends().size();i++)
            result += ContactModel::instance()->backends()[i]->parentBackend()==nullptr?1:0;
         init = true;
      }
      return result;
   }
   else {
      CommonItemBackendModelPrivate::ProxyItem* item = static_cast<CommonItemBackendModelPrivate::ProxyItem*>(parent.internalPointer());
      return item->backend->childrenBackends().size();
   }
}

int CommonItemBackendModel::columnCount (const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return 1+d_ptr->m_lExtensions.size();
}

Qt::ItemFlags CommonItemBackendModel::flags(const QModelIndex& idx) const
{
   if (!idx.isValid())
      return 0;
   CommonItemBackendModelPrivate::ProxyItem* item = static_cast<CommonItemBackendModelPrivate::ProxyItem*>(idx.internalPointer());
   if (idx.column() > 0) {
      //Make sure the cell is disabled if the row is
      Qt::ItemFlags f = d_ptr->m_lExtensions[idx.column()-1]->flags(item->backend,idx);
      return  (((f&Qt::ItemIsEnabled)&&(!item->backend->isEnabled()))?f^Qt::ItemIsEnabled:f);
   }
   const bool checkable = item->backend->supportedFeatures() & (AbstractContactBackend::SupportedFeatures::ENABLEABLE |
   AbstractContactBackend::SupportedFeatures::DISABLEABLE | AbstractContactBackend::SupportedFeatures::MANAGEABLE  );
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | (checkable?Qt::ItemIsUserCheckable:Qt::NoItemFlags);
}

bool CommonItemBackendModel::setData (const QModelIndex& idx, const QVariant &value, int role )
{
   Q_UNUSED(idx)
   Q_UNUSED(value)
   Q_UNUSED(role)
   if (idx.isValid() && idx.column() > 0) {
      CommonItemBackendModelPrivate::ProxyItem* item = static_cast<CommonItemBackendModelPrivate::ProxyItem*>(idx.internalPointer());
      return d_ptr->m_lExtensions[idx.column()-1]->setData(item->backend,idx,value,role);
   }

   if (role == Qt::CheckStateRole && idx.column() == 0) {
      CommonItemBackendModelPrivate::ProxyItem* item = static_cast<CommonItemBackendModelPrivate::ProxyItem*>(idx.internalPointer());
      if (item) {
         const bool old = item->backend->isEnabled();
         ItemModelStateSerializationVisitor::instance()->setChecked(item->backend,value==Qt::Checked);
         emit dataChanged(index(idx.row(),0),index(idx.row(),columnCount()-1));
         if (old != (value==Qt::Checked)) {
            emit checkStateChanged();
         }
         return true;
      }
   }
   return false;
}

QModelIndex CommonItemBackendModel::parent( const QModelIndex& idx ) const
{
   if (idx.isValid()) {
      CommonItemBackendModelPrivate::ProxyItem* item = static_cast<CommonItemBackendModelPrivate::ProxyItem*>(idx.internalPointer());
      if (!item->parent)
         return QModelIndex();
      return createIndex(item->row,item->col,item->parent);
   }
   return QModelIndex();
}

QModelIndex CommonItemBackendModel::index( int row, int column, const QModelIndex& parent ) const
{
   if (parent.isValid()) {
      CommonItemBackendModelPrivate::ProxyItem* parentItem = static_cast<CommonItemBackendModelPrivate::ProxyItem*>(parent.internalPointer());
      CommonItemBackendModelPrivate::ProxyItem* item = nullptr;
      if (row < parentItem->m_Children.size())
         item = parentItem->m_Children[row];
      else {
         item = new CommonItemBackendModelPrivate::ProxyItem();
         item->parent = parentItem;
         item->backend = static_cast<AbstractContactBackend*>(parentItem->backend->childrenBackends()[row]);
         parentItem->m_Children << item;
      }
      item->row    = row;
      item->col    = column;
      return createIndex(row,column,item);
   }
   else { //Top level
      CommonItemBackendModelPrivate::ProxyItem* item = nullptr;
      if (row < d_ptr->m_lTopLevelBackends.size())
         item = d_ptr->m_lTopLevelBackends[row];
      else {
         item = new CommonItemBackendModelPrivate::ProxyItem();
         item->backend = ContactModel::instance()->backends()[row];
         d_ptr->m_lTopLevelBackends << item;
      }
      item->row = row;
      item->col = column;
      return createIndex(item->row,item->col,item);
   }
}

void CommonItemBackendModelPrivate::slotUpdate()
{
   emit q_ptr->layoutChanged();
}

QVariant CommonItemBackendModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      if (section > 0)
         return d_ptr->m_lExtensions[section-1]->headerName();
      return QVariant(tr("Name"));
   }
   return QVariant();
}

bool CommonItemBackendModel::save()
{
   if (ItemModelStateSerializationVisitor::instance()) {

      //Load newly enabled backends
      foreach(CommonItemBackendModelPrivate::ProxyItem* top, d_ptr->m_lTopLevelBackends) {
         AbstractContactBackend* current = top->backend;
         bool check = ItemModelStateSerializationVisitor::instance()->isChecked(current);
         bool wasChecked = current->isEnabled();
         if (check && !wasChecked)
            current->enable(true);
         else if ((!check) && wasChecked)
            current->enable(false);

         //TODO implement real tree digging
         foreach(CommonItemBackendModelPrivate::ProxyItem* leaf ,top->m_Children) {
            current = leaf->backend;
            check = ItemModelStateSerializationVisitor::instance()->isChecked(current);
            wasChecked = current->isEnabled();
            if (check && !wasChecked)
               current->enable(true);
            else if ((!check) && wasChecked)
               current->enable(false);
            //else: do nothing
         }
      }
      return ItemModelStateSerializationVisitor::instance()->save();
   }
   return false;
}

bool CommonItemBackendModel::load()
{
   if (ItemModelStateSerializationVisitor::instance()) {
      return ItemModelStateSerializationVisitor::instance()->load();
   }
   return false;
}

///Return the backend at a given index
AbstractContactBackend* CommonItemBackendModel::backendAt(const QModelIndex& index)
{
   if (!index.isValid())
      return nullptr;
   return static_cast<CommonItemBackendModelPrivate::ProxyItem*>(index.internalPointer())->backend;
}

void CommonItemBackendModel::addExtension(AbstractItemBackendModelExtension* extension)
{
   emit layoutAboutToBeChanged();
   d_ptr->m_lExtensions << extension;
   connect(extension,SIGNAL(dataChanged(QModelIndex)),d_ptr.data(),SLOT(slotExtensionDataChanged(QModelIndex)));
   emit layoutChanged();
}

void CommonItemBackendModelPrivate::slotExtensionDataChanged(const QModelIndex& idx)
{
   emit q_ptr->dataChanged(idx,idx);
}

#include <itembackendmodel.moc>
