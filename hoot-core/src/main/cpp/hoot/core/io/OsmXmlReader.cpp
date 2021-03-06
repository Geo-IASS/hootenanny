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
 * @copyright Copyright (C) 2015, 2016, 2017 DigitalGlobe (http://www.digitalglobe.com/)
 */

#include "OsmXmlReader.h"

#include <cstdlib> // std::system()
#include <cstdio> // for std::remove()

// Boost
using namespace boost;
#include <boost/lexical_cast.hpp>

// Hoot
#include <hoot/core/util/Exception.h>
#include <hoot/core/elements/ConstElementVisitor.h>
#include <hoot/core/elements/Node.h>
#include <hoot/core/elements/Way.h>
#include <hoot/core/elements/Tags.h>
#include <hoot/core/visitors/ReportMissingElementsVisitor.h>
#include <hoot/core/util/ConfigOptions.h>
#include <hoot/core/util/HootException.h>
#include <hoot/core/util/Log.h>
#include <hoot/core/util/MetadataTags.h>
#include <hoot/core/util/OsmUtils.h>
#include <hoot/core/util/Factory.h>
#include <hoot/core/OsmMap.h>

// Qt
#include <QBuffer>
#include <QDateTime>

// Standard
#include <iostream>
using namespace std;

namespace hoot
{
using namespace elements;

unsigned int OsmXmlReader::logWarnCount = 0;

HOOT_FACTORY_REGISTER(OsmMapReader, OsmXmlReader)

OsmXmlReader::OsmXmlReader()
{
  _status = hoot::Status::Invalid;
  _circularError = -1;
  _keepFileStatus = ConfigOptions().getReaderKeepFileStatus();
  _useFileStatus = ConfigOptions().getReaderUseFileStatus();
  _useDataSourceId = false;
  _addSourceDateTime = ConfigOptions().getReaderAddSourceDatetime();
}

void OsmXmlReader::_parseTimeStamp(const QXmlAttributes &attributes)
{
  if ( (attributes.value("timestamp") != "") &&
       (attributes.value("timestamp") != "1970-01-01T00:00:00Z") &&
       (_addSourceDateTime == true) )
  {
    _element->setTag(MetadataTags::SourceDateTime(),attributes.value("timestamp"));
  }

}

void OsmXmlReader::_createNode(const QXmlAttributes &attributes)
{
  long id = _parseLong(attributes.value("id"));
  long newId;
  if (_useDataSourceId)
  {
    newId = id;
  }
  else
  {
    newId = _map->createNextNodeId();
  }
  _nodeIdMap.insert(id, newId);
  double x = _parseDouble(attributes.value("lon"));
  double y = _parseDouble(attributes.value("lat"));

  // check the next 3 attributes to see if a value exist, if not, assign a default since these
  // are not officially required by the DTD
  long version = ElementData::VERSION_EMPTY;
  if (attributes.value("version") != "")
  {
    version = _parseDouble(attributes.value("version"));
  }
  long changeset = ElementData::CHANGESET_EMPTY;
  if (attributes.value("changeset") != "")
  {
    changeset = _parseDouble(attributes.value("changeset"));
  }
  unsigned int timestamp = ElementData::TIMESTAMP_EMPTY;
  if (attributes.value("timestamp") != "")
  {
    timestamp = OsmUtils::fromTimeString(attributes.value("timestamp"));
  }
  QString user = ElementData::USER_EMPTY;
  if (attributes.value("user") != "")
  {
    user = attributes.value("user");
  }
  long uid = ElementData::UID_EMPTY;
  if (attributes.value("uid") != "")
  {
    uid = _parseDouble(attributes.value("uid"));
  }

  _element.reset(new Node(_status, newId, x, y, _circularError, changeset, version, timestamp,
                          user, uid));

  if (_element->getTags().getInformationCount() > 0)
  {
    _parseTimeStamp(attributes);
  }
}

void OsmXmlReader::_createRelation(const QXmlAttributes &attributes)
{
  _relationId = _parseLong(attributes.value("id"));
  long newId = _getRelationId(_relationId);

  // check the next 3 attributes to see if a value exist, if not, assign a default since these are
  // not officially required by the DTD
  long version = ElementData::VERSION_EMPTY;
  if (attributes.value("version") != "")
  {
    version = _parseDouble(attributes.value("version"));
  }
  long changeset = ElementData::CHANGESET_EMPTY;
  if (attributes.value("changeset") != "")
  {
    changeset = _parseDouble(attributes.value("changeset"));
  }
  unsigned int timestamp = ElementData::TIMESTAMP_EMPTY;
  if (attributes.value("timestamp") != "")
  {
    timestamp = OsmUtils::fromTimeString(attributes.value("timestamp"));
  }
  QString user = ElementData::USER_EMPTY;
  if (attributes.value("user") != "")
  {
    user = attributes.value("user");
  }
  long uid = ElementData::UID_EMPTY;
  if (attributes.value("uid") != "")
  {
    uid = _parseDouble(attributes.value("uid"));
  }

  _element.reset(new Relation(_status, newId, _circularError, "", changeset, version, timestamp,
                              user, uid));

  _parseTimeStamp(attributes);
}

void OsmXmlReader::_createWay(const QXmlAttributes &attributes)
{
  _wayId = _parseLong(attributes.value("id"));

  long newId;
  if (_useDataSourceId)
  {
    newId = _wayId;
  }
  else
  {
    newId = _map->createNextWayId();
  }
  _wayIdMap.insert(_wayId, newId);

  // check the next 3 attributes to see if a value exist, if not, assign a default since these are not officially required by the DTD
  // check the next 3 attributes to see if a value exist, if not, assign a default since these are not officially required by the DTD
  long version = ElementData::VERSION_EMPTY;
  if (attributes.value("version") != "")
  {
    version = _parseDouble(attributes.value("version"));
  }
  long changeset = ElementData::CHANGESET_EMPTY;
  if (attributes.value("changeset") != "")
  {
    changeset = _parseDouble(attributes.value("changeset"));
  }
  unsigned int timestamp = ElementData::TIMESTAMP_EMPTY;
  if (attributes.value("timestamp") != "")
  {
    timestamp = OsmUtils::fromTimeString(attributes.value("timestamp"));
  }
  QString user = ElementData::USER_EMPTY;
  if (attributes.value("user") != "")
  {
    user = attributes.value("user");
  }
  long uid = ElementData::UID_EMPTY;
  if (attributes.value("uid") != "")
  {
    uid = _parseDouble(attributes.value("uid"));
  }

  _element.reset(new Way(_status, newId, _circularError, changeset, version, timestamp, user,
                         uid));

  _parseTimeStamp(attributes);
}

bool OsmXmlReader::fatalError(const QXmlParseException &exception)
{
  _errorString = QObject::tr("Parse error at line %1, column %2:\n%3")
      .arg(exception.lineNumber())
      .arg(exception.columnNumber())
      .arg(exception.message());
  return false;
}

bool OsmXmlReader::isSupported(QString url)
{
  const int numExtensions = 3;
  const QString validExtensions[numExtensions] = { ".osm", ".osm.bz2", ".osm.gz" };
  const QString checkString( url.toLower() );

  // Ticket 5871: support compressed osm files
  for ( int i = 0; i < numExtensions; ++ i )
  {
    if ( checkString.endsWith(validExtensions[i]) == true )
    {
      return true;
    }
  }

  // If we fall out of loop, no dice
  return false;
}

double OsmXmlReader::_parseDouble(QString s)
{
  bool ok;
  double result = s.toDouble(&ok);

  if (ok == false)
  {
      throw Exception("Error parsing double: " + s);
  }

  return result;
}

long OsmXmlReader::_parseLong(QString s)
{
  bool ok;
  long result = s.toLong(&ok);

  if (ok == false)
  {
      throw Exception("Error parsing long: " + s);
  }

  return result;
}

int OsmXmlReader::_parseInt(QString s)
{
  bool ok;
  int result = s.toInt(&ok);

  if (ok == false)
  {
      throw Exception("Error parsing int: " + s);
  }

  return result;
}

void OsmXmlReader::open(QString url)
{
  _path = url;
}

void OsmXmlReader::read(OsmMapPtr map)
{
  _osmFound = false;

  _missingNodeCount = 0;
  _missingWayCount = 0;
  _badAccuracyCount = 0;
  _map = map;

  // Ticket 5871: uncompress .osm.bz2 or .osm.gz files before processing
  QString originalFile;
  if ( _path.endsWith(".osm.bz2") == true )
  {
    originalFile = _path;
    _path.chop(std::strlen(".bz2"));

    // "man bunzip2" confirms success return code is zero
    // -f option decompresses file even if decompressed file is already there
    // -k option is "keep", meaning don't delete input .osm.bz2
    const std::string cmd(std::string("bunzip2 -fk ") + originalFile.toStdString());
    LOG_DEBUG("Running uncompress command: " << cmd);

    int retVal;
    if ( (retVal = std::system(cmd.c_str())) != 0 )
    {
      QString error = QString("Error %1 returned from uncompress command: %2").arg(retVal).
        arg(QString::fromStdString(cmd));
      throw HootException(error);
    }

    LOG_DEBUG("Uncompress succeeded!");
  }
  else if ( _path.endsWith(".osm.gz") == true )
  {
    originalFile = _path;
    _path.chop(std::strlen(".gz"));

    // "man gzip" confirms success return code is zero
    //  -d option is "decompress"
    //  -k option is "keep," meaning don't delete input .osm.gz
    const std::string cmd(std::string("gzip -d -k ") + originalFile.toStdString());
    LOG_DEBUG("Running uncompress command: " << cmd);

    int retVal;
    if ( (retVal = std::system(cmd.c_str())) != 0 )
    {
      QString error = QString("Error %1 returned from uncompress command: %2").arg(retVal).
        arg(QString::fromStdString(cmd));
      throw HootException(error);
    }

    LOG_DEBUG("Uncompress succeeded!");
  }

  // do xml parsing
  QXmlSimpleReader reader;
  reader.setContentHandler(this);
  reader.setErrorHandler(this);

  QFile file(_path);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
      throw Exception(QObject::tr("Error opening OSM file for parsing: %1").arg(_path));
  }
  LOG_DEBUG("File " << _path << " opened for read");

  QXmlInputSource xmlInputSource(&file);

  if (reader.parse(xmlInputSource) == false)
  {
      throw HootException(_errorString);
  }
  file.close();

  // Ticket 5871: if we did have to decompress, delete the decompressed file when we're done
  if ( originalFile.length() > 0 )
  {
    // Delete the temp file
    std::remove(_path.toStdString().c_str());
    LOG_DEBUG("Removed decompressed file " << _path);
  }

  ReportMissingElementsVisitor visitor;
  _map->visitRw(visitor);

  _map.reset();
}

void OsmXmlReader::readFromString(QString xml, OsmMapPtr map)
{
  _osmFound = false;

  _missingNodeCount = 0;
  _missingWayCount = 0;
  _badAccuracyCount = 0;
  _map = map;

  // do xml parsing
  QXmlSimpleReader reader;
  reader.setContentHandler(this);
  reader.setErrorHandler(this);

  QBuffer buffer;
  buffer.setData(xml.toUtf8());

  QXmlInputSource xmlInputSource(&buffer);
  if (reader.parse(xmlInputSource) == false)
  {
    throw Exception(_errorString);
  }

  ReportMissingElementsVisitor visitor;
  _map->visitRw(visitor);

  _map.reset();
}

void OsmXmlReader::read(const QString& path, OsmMapPtr map)
{
  open(path);
  read(map);
}

const QString& OsmXmlReader::_saveMemory(const QString& s)
{
  if (!_strings.contains(s))
  {
    _strings[s] = s;
  }
  return _strings[s];
}

bool OsmXmlReader::startElement(const QString & /* namespaceURI */,
                             const QString & /* localName */,
                             const QString &qName,
                             const QXmlAttributes &attributes)
{
  try
  {
      if (!_osmFound)
      {
          if (qName != "osm")
          {
              throw Exception("The file is not an OSM file.");
          }
          else
          {
              _osmFound = true;
              if (attributes.value("version") != "0.6")
              {
                  throw Exception("Only version 0.6 OSM files are supported.");
              }
          }
      }

      if (qName == "node")
      {
        if (attributes.value("action") != "delete")
        {
          _createNode(attributes);
        }
        else
        {
          _element.reset();
        }
      }
      else if (qName == "way")
      {
        if (attributes.value("action") != "delete")
        {
          _createWay(attributes);
        }
        else
        {
          _element.reset();
        }
      }
      else if (qName == "relation")
      {
        if (attributes.value("action") != "delete")
        {
          _createRelation(attributes);
        }
        else
        {
          _element.reset();
        }
      }
      else if (qName == "nd" && _element)
      {
        long ref = _parseLong(attributes.value("ref"));

        if (_nodeIdMap.contains(ref) == false)
        {
          _missingNodeCount++;
          if (logWarnCount < ConfigOptions().getLogWarnMessageLimit())
          {
            LOG_WARN("Missing node (" << ref << ") in way (" << _wayId << ").");
          }
          else if (logWarnCount == ConfigOptions().getLogWarnMessageLimit())
          {
            LOG_WARN(className() << ": " << Log::LOG_WARN_LIMIT_REACHED_MESSAGE);
          }
          logWarnCount++;
        }
        else
        {
          long newRef = _nodeIdMap.value(ref);

          WayPtr w = boost::dynamic_pointer_cast<Way, Element>(_element);

          w->addNode(newRef);
        }
      }
      else if (qName == "member" && _element)
      {
        long ref = _parseLong(attributes.value("ref"));
        QString type = attributes.value("type");
        QString role = attributes.value("role");

        RelationPtr r = boost::dynamic_pointer_cast<Relation, Element>(_element);

        if (type == "node")
        {
          if (_nodeIdMap.contains(ref) == false)
          {
            _missingNodeCount++;
            if (logWarnCount < ConfigOptions().getLogWarnMessageLimit())
            {
              LOG_WARN("Missing node (" << ref << ") in relation (" << _relationId << ").");
            }
            else if (logWarnCount == ConfigOptions().getLogWarnMessageLimit())
            {
              LOG_WARN(className() << ": " << Log::LOG_WARN_LIMIT_REACHED_MESSAGE);
            }
            logWarnCount++;
          }
          else
          {
            long newRef = _nodeIdMap.value(ref);
            r->addElement(role, ElementType::Node, newRef);
          }
        }
        else if (type == "way")
        {
          if (_wayIdMap.contains(ref) == false)
          {
            _missingWayCount++;
            if (logWarnCount < ConfigOptions().getLogWarnMessageLimit())
            {
              LOG_WARN("Missing way (" << ref << ") in relation (" << _relationId << ").");
            }
            else if (logWarnCount == ConfigOptions().getLogWarnMessageLimit())
            {
              LOG_WARN(className() << ": " << Log::LOG_WARN_LIMIT_REACHED_MESSAGE);
            }
            logWarnCount++;
          }
          else
          {
            long newRef = _wayIdMap.value(ref);
            r->addElement(role, ElementType::Way, newRef);
          }
        }
        else if (type == "relation")
        {
          // relations may be out of order so we don't check for consistency at this stage.
          long newRef = _getRelationId(ref);
          r->addElement(role, ElementType::Relation, newRef);
        }
        else
        {
          if (logWarnCount < ConfigOptions().getLogWarnMessageLimit())
          {
            LOG_WARN("Found a relation member with unexpected type: " << type << " in relation ("
                     << _relationId << ")");
          }
          else if (logWarnCount == ConfigOptions().getLogWarnMessageLimit())
          {
            LOG_WARN(className() << ": " << Log::LOG_WARN_LIMIT_REACHED_MESSAGE);
          }
          logWarnCount++;
        }
      }
      else if (qName == "tag" && _element)
      {
        const QString& key = _saveMemory(attributes.value("k"));
        const QString& value = _saveMemory(attributes.value("v"));

        if (_useFileStatus && key == MetadataTags::HootStatus())
        {
          _element->setStatus(Status::fromString(value));

          if (_keepFileStatus)  { _element->setTag(key, value); }
        }
        else if (key == "type" && _element->getElementType() == ElementType::Relation)
        {
          RelationPtr r = boost::dynamic_pointer_cast<Relation, Element>(_element);
          r->setType(value);

          if (ConfigOptions().getReaderPreserveAllTags()) { _element->setTag(key, value); }
        }
        else if (key == MetadataTags::Accuracy() || key == MetadataTags::ErrorCircular())
        {
          bool ok;
          Meters circularError = value.toDouble(&ok);

          if (circularError > 0 && ok)
          {
            _element->setCircularError(circularError);
          }
          else
          {
            bool isBad = false;
            hoot::Tags t1;
            t1.set(key, value);
            try
            {
              circularError = t1.getLength(key).value();
              if (circularError > 0)
              {
                _element->setCircularError(circularError);
              }
              else
              {
                isBad = true;
              }
            }
            catch (const HootException&)
            {
              isBad = true;
            }

            if (isBad)
            {
              _badAccuracyCount++;
              if (logWarnCount < ConfigOptions().getLogWarnMessageLimit())
              {
                LOG_WARN("Bad circular error value: " << value.toStdString());
              }
              else if (logWarnCount == ConfigOptions().getLogWarnMessageLimit())
              {
                LOG_WARN(className() << ": " << Log::LOG_WARN_LIMIT_REACHED_MESSAGE);
              }
              logWarnCount++;
            }
          }
          if (ConfigOptions().getReaderPreserveAllTags())
          {
            _element->setTag(key, value);
          }
        }
        else
        {
          if (key != MetadataTags::HootId() && value != "")
          {
            _element->setTag(key, value);
          }
        }
      }
  }
  catch (const Exception& e)
  {
      _errorString = e.what();
      return false;
  }

  return true;
}

bool OsmXmlReader::endElement(const QString & /* namespaceURI */,
                             const QString & /* localName */,
                             const QString &qName)
{
  if (_element)
  {
    if (qName == "node")
    {
        NodePtr n = boost::dynamic_pointer_cast<Node, Element>(_element);
        _map->addNode(n);
    }
    else if (qName == "way")
    {
        WayPtr w = boost::dynamic_pointer_cast<Way, Element>(_element);
        _map->addWay(w);
    }
    else if (qName == "relation")
    {
        RelationPtr r = boost::dynamic_pointer_cast<Relation, Element>(_element);
        _map->addRelation(r);
    }
  }

  return true;
}

long OsmXmlReader::_getRelationId(long fileId)
{
  long newId;
  if (_useDataSourceId)
  {
    newId = fileId;
    _relationIdMap.insert(fileId, newId);
  }
  else
  {
    if (_relationIdMap.find(fileId) == _relationIdMap.end())
    {
      newId = _map->createNextRelationId();
      _relationIdMap.insert(fileId, newId);
    }
    else
    {
      newId = _relationIdMap[fileId];
    }
  }
  return newId;
}


}
