/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                         ***
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
#ifndef PRESENCESERIALIZATIONVISITOR_H
#define PRESENCESERIALIZATIONVISITOR_H

#include "../typedefs.h"

class AbstractBookmarkModel;
class AbstractItemBackendBase;

class LIB_EXPORT PresenceSerializationVisitor {
public:
   virtual void serialize() = 0;
   virtual void load     () = 0;
   virtual bool isTracked(AbstractItemBackendBase* backend) = 0;
   virtual void setTracked(AbstractItemBackendBase* backend, bool tracked) = 0;
   virtual ~PresenceSerializationVisitor(){};

   static PresenceSerializationVisitor* instance();
   static void setInstance(PresenceSerializationVisitor* ins);
private:
   static PresenceSerializationVisitor* m_spInstance;
};

#endif //PRESENCESERIALIZATIONVISITOR_H
