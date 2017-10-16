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
 * @copyright Copyright (C) 2017 DigitalGlobe (http://www.digitalglobe.com/)
 */
#include "ImplicitTagRulesTsvWriter.h"

// hoot
#include <hoot/core/util/HootException.h>
#include <hoot/core/util/Factory.h>

// Qt
#include <QStringBuilder>

namespace hoot
{

HOOT_FACTORY_REGISTER(ImplicitTagRuleWordPartWriter, ImplicitTagRulesTsvWriter)

ImplicitTagRulesTsvWriter::ImplicitTagRulesTsvWriter()
{
}

ImplicitTagRulesTsvWriter::~ImplicitTagRulesTsvWriter()
{
  close();
}

bool ImplicitTagRulesTsvWriter::isSupported(const QString url)
{
  return url.endsWith(".tsv", Qt::CaseInsensitive);
}

void ImplicitTagRulesTsvWriter::open(const QString url)
{
  _file.reset(new QFile());
  _file->setFileName(url);
  if (_file->exists() && !_file->remove())
  {
    throw HootException(QObject::tr("Error removing existing %1 for writing.").arg(url));
  }
  if (!_file->open(QIODevice::WriteOnly | QIODevice::Text))
  {
    throw HootException(QObject::tr("Error opening %1 for writing.").arg(url));
  }
  LOG_DEBUG("Opened: " << url << ".");
}

void ImplicitTagRulesTsvWriter::write(const ImplicitTagRuleWordPart& ruleWordPart,
                                      const long /*totalParts*/)
{
  //each word takes up two rows; first col in first row contains words; remaining cols in first row
  //contain kvps; in second row, each kvp has the count directly below it

  const QString word = ruleWordPart.getWord();
  QString row1 = word % "\t";
  QString row2 = "\t";
  const QMap<QString, long> kvpsWithCount = ruleWordPart.getTagsToCounts();
  for (QMap<QString, long>::const_iterator kvpItr = kvpsWithCount.begin();
       kvpItr != kvpsWithCount.end(); ++kvpItr)
  {
    row1 += kvpItr.key() % "\t";
    row2 += QString::number(kvpItr.value()) % "\t";
  }
  row1.chop(1);
  row1 += "\n";
  _file->write(row1.toUtf8());
  row2.chop(1);
  row2 += "\n";
  _file->write(row2.toUtf8());
}

void ImplicitTagRulesTsvWriter::close()
{
  if (_file.get())
  {
    _file->close();
    _file.reset();
  }
}

}
