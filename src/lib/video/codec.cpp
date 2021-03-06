/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
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
#include "codec.h"
#include "../private/videocodec_p.h"

#include <account.h>


QHash<QString,Video::Codec*> VideoCodecPrivate::m_slCodecs;
bool VideoCodecPrivate::m_sInit = false;

VideoCodecPrivate::VideoCodecPrivate() : m_Enabled(false),m_Bitrate(0)
{
}

///Private constructor
Video::Codec::Codec(const QString &codecName, uint bitRate, bool enabled) : QObject(nullptr),
d_ptr(new VideoCodecPrivate())
{
   d_ptr->m_Name    = codecName;
   d_ptr->m_Bitrate = bitRate  ;
   d_ptr->m_Enabled = enabled  ;

   setObjectName("VideoCodec"+codecName);
}

Video::Codec::~Codec()
{
   delete d_ptr;
}

///Get the current codec name
QString Video::Codec::name() const
{
   return d_ptr->m_Name;
}

///Get the current codec id
uint Video::Codec::bitrate() const
{
   return d_ptr->m_Bitrate;
}

///Get the current codec id
bool Video::Codec::isEnabled() const
{
   return d_ptr->m_Enabled;
}

///Set the codec bitrate
void Video::Codec::setBitrate(const uint bitrate)
{
   d_ptr->m_Bitrate = bitrate;
}

///Set if the codec is enabled
void Video::Codec::setEnabled(const bool enabled)
{
   d_ptr->m_Enabled = enabled;
}

///Set codec parameters
void Video::Codec::setParamaters(const QString& params )
{
   d_ptr->m_Parameters = params;
}

///Get codec parameters
QString Video::Codec::parameters() const
{
   return d_ptr->m_Parameters;
}

///Generate a daemon compatible codec representation
QMap<QString,QString> Video::Codec::toMap() const
{
   QMap<QString,QString> ret;
   ret[VideoCodecPrivate::CodecFields::ENABLED    ] = isEnabled ()?"true":"false";
   ret[VideoCodecPrivate::CodecFields::BITRATE    ] = QString::number(bitrate());
   ret[VideoCodecPrivate::CodecFields::NAME       ] = name      ();
   ret[VideoCodecPrivate::CodecFields::PARAMETERS ] = parameters();
   return ret;
}
