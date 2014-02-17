/************************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                                       *
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
#ifndef MINIMALHISTORYBACKEND_H
#define MINIMALHISTORYBACKEND_H

#include "../lib/abstractitembackend.h"

class LIB_EXPORT MinimalHistoryBackend : public AbstractHistoryBackend
{
public:
   explicit MinimalHistoryBackend(QObject* parent = nullptr);
   virtual ~MinimalHistoryBackend();

   virtual bool load();
   virtual bool reload();
   virtual bool save(const Call* call);

   virtual SupportedFeatures  supportedFeatures() const;

   ///Edit 'item', the implementation may be a GUI or somehting else
   virtual bool edit( Call* call);
   ///Add a new item to the backend
   virtual bool addNew( Call* call);

   ///Add a new phone number to an existing item
   virtual bool addPhoneNumber( Call* call , PhoneNumber* number );
};

#endif