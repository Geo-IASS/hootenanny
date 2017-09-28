/*
 * This file is part of Hootenanny.
 *
 * Hootenanny is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * The following copyright notices are generated automatically. If you
 * have a new notice to add, please use the format:
 * " * @copyright Copyright ..."
 * This will properly maintain the copyright information. DigitalGlobe
 * copyrights will be updated automatically.
 *
 * @copyright Copyright (C) 2015 DigitalGlobe (http://www.digitalglobe.com/)
 */

#include "ImplicitTagRulesSqlLiteReader.h"

// hoot
#include <hoot/core/util/HootException.h>
#include <hoot/core/util/Log.h>

// Qt
#include <QSqlError>
#include <QVariant>

namespace hoot
{

ImplicitTagRulesSqlLiteReader::ImplicitTagRulesSqlLiteReader(const QString path)
{
  if (QSqlDatabase::contains(path) == false)
  {
    _db = QSqlDatabase::addDatabase("QSQLITE", path);
    _db.setDatabaseName(path);
    if (_db.open() == false)
    {
      throw HootException("Error opening DB. " + path);
    }
  }
  else
  {
    _db = QSqlDatabase::database(path);
  }

  if (_db.isOpen() == false)
  {
    throw HootException("Error DB is not open. " + path);
  }

  _select = QSqlQuery(_db);
  if (_select.prepare("SELECT kvp FROM rules WHERE word=:word") == false)
  {
    throw HootException(QString("Error preparing query: %1").arg(_select.lastError().text()));
  }

}

Tags ImplicitTagRulesSqlLiteReader::getTags(const QString word) const
{
  _select.bindValue(":word", word);

  if (_select.exec() == false)
  {
    throw HootException(QString("Error executing query: %1").arg(_select.lastError().text()));
  }

  Tags tags;
  while (_select.next())
  {
    tags.appendValue(_select.value(0).toString());
  }

  return tags;
}

}
