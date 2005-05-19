/* 
 *  Copyright (C) 2004-2005 Christian Loose <christian.loose@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef CERVISIA_IGNOREFILTERBASE_H
#define CERVISIA_IGNOREFILTERBASE_H

class QFileInfo;
class QString;


namespace Cervisia
{


class IgnoreFilterBase
{
public:
    IgnoreFilterBase(IgnoreFilterBase* nextFilter=0);
    virtual ~IgnoreFilterBase();

    bool matches(const QString& fileName) const;

protected:
    void addEntriesFromString(const QString& str);
    void addEntriesFromFile(const QString& name);

private:
    virtual void addEntry(const QString& entry) = 0;
    virtual bool doMatches(const QString& fileName) const = 0;

    IgnoreFilterBase* m_nextFilter;
};


}


#endif
