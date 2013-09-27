/****************************************************************************
 *   Copyright (C) 2013 by Savoir-Faire Linux                               *
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
#include "numbercategorymodel.h"
#include "visitors/numbercategoryvisitor.h"

NumberCategoryModel* NumberCategoryModel::m_spInstance = nullptr;

NumberCategoryModel::NumberCategoryModel(QObject* parent) : QAbstractListModel(parent),m_pVisitor(nullptr)
{
   
}

//Abstract model member
QVariant NumberCategoryModel::data(const QModelIndex& index, int role) const
{
   if (!index.isValid()) return QVariant();
   switch (role) {
      case Qt::DisplayRole:
         return m_lCategories[index.row()]->name;
      case Qt::DecorationRole:
         if (m_lCategories[index.row()]->icon && m_pVisitor)
            return m_pVisitor->icon(m_lCategories[index.row()]->icon);
      case Qt::CheckStateRole:
         return m_lCategories[index.row()]->enabled?Qt::Checked:Qt::Unchecked;
      case Role::INDEX:
         return m_lCategories[index.row()]->index;
   }
   return QVariant();
}

int NumberCategoryModel::rowCount(const QModelIndex& parent) const
{
   if (parent.isValid()) return 0;
   return m_lCategories.size();
}

Qt::ItemFlags NumberCategoryModel::flags(const QModelIndex& index) const
{
   Q_UNUSED(index)
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

bool NumberCategoryModel::setData(const QModelIndex& idx, const QVariant &value, int role)
{
   if (idx.isValid() && role == Qt::CheckStateRole) {
      m_lCategories[idx.row()]->enabled = value.toBool();
      emit dataChanged(idx,idx);
      return true;
   }
   return false;
}

void NumberCategoryModel::addCategory(const QString& name, QPixmap* icon, int index, bool enabled)
{
   InternalTypeRepresentation* rep = new InternalTypeRepresentation();
   rep->name       = name   ;
   rep->icon       = icon   ;
   rep->index      = index  ;
   rep->enabled    = enabled;
   m_hByIdx[index] = rep    ;
   m_lCategories  << rep    ;
   emit layoutChanged()     ;
}

NumberCategoryModel* NumberCategoryModel::instance()
{
   if (!m_spInstance)
      m_spInstance = new NumberCategoryModel();
   return m_spInstance;
}

void NumberCategoryModel::setIcon(int idx, QPixmap* icon)
{
   InternalTypeRepresentation* rep = m_hByIdx[idx];
   if (rep) {
      rep->icon = icon;
      emit dataChanged(index(m_lCategories.indexOf(rep),0),index(m_lCategories.indexOf(rep),0));
   }
}

void NumberCategoryModel::setVisitor(NumberCategoryVisitor* visitor)
{
   m_pVisitor = visitor;
   m_pVisitor->load(this);
}

void NumberCategoryModel::save()
{
   if (m_pVisitor) {
      m_pVisitor->serialize(this);
   }
   else
      qDebug() << "Cannot save NumberCategoryModel as there is no defined backend";
}