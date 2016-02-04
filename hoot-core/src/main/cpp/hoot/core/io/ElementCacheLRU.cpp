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
 */

#include <map>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <hoot/core/elements/Element.h>
#include <hoot/core/elements/ElementType.h>
#include <hoot/core/elements/Node.h>
#include <hoot/core/elements/Way.h>
#include <hoot/core/elements/Relation.h>
#include <hoot/core/util/Log.h>

#include "ElementCacheLRU.h"

namespace hoot
{

ElementCacheLRU::ElementCacheLRU(const unsigned long maxCountEachElementType) :
  _maxCountPerType(maxCountEachElementType),
  _projection(),
  _nodes(),
  _nodesIter(_nodes.begin()),
  _ways(),
  _waysIter(_ways.begin()),
  _relations(),
  _relationsIter(_relations.begin())
{
  LOG_DEBUG("New LRU cache created, " << _maxCountPerType << " entries for each type");
}

bool ElementCacheLRU::isEmpty() const
{
  return ( _nodes.empty() && _ways.empty() && _relations.empty() );
}

unsigned long ElementCacheLRU::size() const
{
  return ( _nodes.size() + _ways.size() + _relations.size() );
}


void ElementCacheLRU::addElement(ConstElementPtr &newElement)
{
  ConstNodePtr newNode;
  ConstWayPtr newWay;
  ConstRelationPtr newRelation;

  switch ( newElement->getElementType().getEnum() )
  {
  case ElementType::Node:
    newNode = dynamic_pointer_cast<const Node>(newElement);
    if ( newNode != ConstNodePtr() )
    {
      // Do we have to replace an entry?
      if ( _nodes.size() == _maxCountPerType )
      {
        _removeOldest(ElementType::Node);
      }

      _nodes.insert(std::make_pair(newNode->getId(),
       std::make_pair(newNode, boost::posix_time::microsec_clock::universal_time())));
        //LOG_DEBUG("Added new node with ID " << newNode->getId() );
    }
    break;
  case ElementType::Way:
    newWay = dynamic_pointer_cast<const Way>(newElement);
    if ( newWay != ConstWayPtr() )
    {
      // Do we have to replace an entry?
      if ( _ways.size() == _maxCountPerType )
      {
        _removeOldest(ElementType::Way);
      }
      _ways.insert(std::make_pair(newWay->getId(),
        std::make_pair(newWay, boost::posix_time::microsec_clock::universal_time())));
    }

    break;
  case ElementType::Relation:
    newRelation = dynamic_pointer_cast<const Relation>(newElement);
    if ( newRelation != ConstRelationPtr() )
    {
      // Do we have to replace an entry?
      if ( _relations.size() == _maxCountPerType )
      {
        _removeOldest(ElementType::Relation);
      }

      _relations.insert(std::make_pair(newRelation->getId(),
        std::make_pair(newRelation, boost::posix_time::microsec_clock::universal_time())));
    }
    break;
  default:
    throw HootException(QString("Unexpected element type: %1").arg(
                          newElement->getElementType().toString()));

    break;
  }

  // Reset all iterators to maintain interface contract
  resetElementIterators();
}

ConstNodePtr ElementCacheLRU::getNextNode()
{
  ConstNodePtr returnPtr;

  if ( _nodesIter != _nodes.end() )
  {
    _nodesIter->second.second = boost::posix_time::microsec_clock::universal_time();
    returnPtr = _nodesIter->second.first;
    _nodesIter++;
  }

  return returnPtr;
}

void ElementCacheLRU::close()
{
  _nodes.clear();
  _ways.clear();
  _relations.clear();

  resetElementIterators();
}

bool ElementCacheLRU::hasMoreElements()
{
  return (
        (_nodesIter != _nodes.end()) ||
        (_waysIter != _ways.end()) ||
        (_relationsIter != _relations.end()) );
}

ElementPtr ElementCacheLRU::readNextElement()
{
  ElementPtr returnElement;

  if ( hasMoreElements() == true )
  {
    if ( _nodesIter != _nodes.end() )
    {
      *returnElement = *getNextNode();
    }
    else if ( _waysIter != _ways.end() )
    {
      *returnElement = *getNextWay();
    }
    else
    {
      *returnElement = *getNextRelation();
    }
  }

  return returnElement;
}

/*
const RelationPtr& ElementCacheLRU::getRelation(long relationId)
{
  const RelationPtr returnRelation;
  std::map<long,
    std::pair<ConstRelationPtr, boost::posix_time::ptime> >::iterator searchIter;

  if ( (searchIter = _relations.find(relationId)) != _relations.end() )
  {
    // Update access time
    searchIter->second.second = boost::posix_time::microsec_clock::universal_time();

    const RelationPtr foundRelation(searchIter->second.first);
    return foundRelation;
  }

  return returnRelation;
}
*/

void ElementCacheLRU::writeElement(ElementInputStream& inputStream)
{
  boost::shared_ptr<OGRSpatialReference> emptyProjection;

  // Find out if we need to set our projection
  if ( _projection == emptyProjection )
  {
    _projection = inputStream.getProjection();
  }
  else
  {
    // Make sure they haven't shifted projections on us
    if ( inputStream.getProjection()->IsSame(&(*_projection)) == false )
    {
      LOG_ERROR("Tried to change projections in mid-stream");
      return;
    }
  }
  ConstElementPtr newElement = inputStream.readNextElement();
  addElement(newElement);
}

void ElementCacheLRU::writeElement(ElementPtr &element)
{
  //  Check projection
  ConstElementPtr el = element;
  addElement(el);
}

/*
ConstWayPtr ElementCacheLRU::getWay(long wayId)
{
  ConstWayPtr returnWay;
  std::map<long,
    std::pair<ConstWayPtr, boost::posix_time::ptime> >::iterator searchIter;

  if ( (searchIter = _ways.find(wayId)) != _ways.end() )
  {
    // Update access time
    searchIter->second.second = boost::posix_time::microsec_clock::universal_time();

    returnWay = searchIter->second.first;
  }

  return returnWay;
}
*/

ConstWayPtr ElementCacheLRU::getNextWay()
{
  ConstWayPtr returnPtr;

  if ( _waysIter != _ways.end() )
  {
    _waysIter->second.second = boost::posix_time::microsec_clock::universal_time();
    returnPtr = _waysIter->second.first;
    _waysIter++;
  }

  return returnPtr;
}

ConstRelationPtr ElementCacheLRU::getNextRelation()
{
  ConstRelationPtr returnPtr;

  if ( _relationsIter != _relations.end() )
  {
    _relationsIter->second.second = boost::posix_time::microsec_clock::universal_time();
    returnPtr = _relationsIter->second.first;
    _relationsIter++;
  }

  return returnPtr;
}

void ElementCacheLRU::resetElementIterators()
{
  _nodesIter = _nodes.begin();
  _waysIter = _ways.begin();
  _relationsIter = _relations.begin();
}

/*
ConstNodePtr ElementCacheLRU::getNode(long nodeId)
{
  ConstNodePtr returnNode;
  std::map<long,
    std::pair<ConstNodePtr, boost::posix_time::ptime> >::iterator searchIter;

  if ( (searchIter = _nodes.find(nodeId)) != _nodes.end() )
  {
    // Update access time
    searchIter->second.second = boost::posix_time::microsec_clock::universal_time();

    returnNode = searchIter->second.first;
  }

  return returnNode;
}
*/

void ElementCacheLRU::_removeOldest(const ElementType::Type typeToRemove)
{
  std::map<long, std::pair<ConstNodePtr, boost::posix_time::ptime> >::iterator nodesIter;
  std::map<long, std::pair<ConstWayPtr, boost::posix_time::ptime> >::iterator waysIter;
  std::map<long, std::pair<ConstRelationPtr, boost::posix_time::ptime> >::iterator relationsIter;

  // Initialize oldest to current, then we can walk it back
  boost::posix_time::ptime oldestTime(boost::posix_time::microsec_clock::universal_time());
  long oldestId = 0;

  switch ( typeToRemove )
  {
  case ElementType::Node:
    for (nodesIter = _nodes.begin(); nodesIter != _nodes.end(); nodesIter++)
    {
      if ( nodesIter->second.second < oldestTime )
      {
        oldestTime = nodesIter->second.second;
        oldestId = nodesIter->first;
      }
    }

    // Remove oldest entry
    _nodes.erase(oldestId);

    break;

  case ElementType::Way:
    for (waysIter = _ways.begin(); waysIter != _ways.end(); waysIter++)
    {
      if ( waysIter->second.second < oldestTime )
      {
        oldestTime = waysIter->second.second;
        oldestId = waysIter->first;
      }
    }

    // Remove oldest entry
    _ways.erase(oldestId);

    break;

  case ElementType::Relation:
    for (relationsIter = _relations.begin(); relationsIter != _relations.end(); relationsIter++)
    {
      if ( relationsIter->second.second < oldestTime )
      {
        oldestTime = relationsIter->second.second;
        oldestId = relationsIter->first;
      }
    }

    // Remove oldest entry
    _relations.erase(oldestId);

    break;

  // Intentional fallthrow
  case ElementType::Unknown:
  default:
    throw HootException(QString("Tried to remove oldest of invalid type"));

    break;
  }
}

boost::shared_ptr<OGRSpatialReference> ElementCacheLRU::getProjection() const
{
  return _projection;
}

bool ElementCacheLRU::containsElement(const ElementId& eid) const
{
  const ElementType::Type type = eid.getType().getEnum();
  const long id = eid.getId();

  switch ( type )
  {
  case ElementType::Node:
    if ( _nodes.find(id) != _nodes.end() )
    {
      return true;
    }

    break;

  case ElementType::Way:
    if ( _ways.find(id) != _ways.end() )
    {
      return true;
    }

    break;

  case ElementType::Relation:
    if ( _relations.find(id) != _relations.end() )
    {
      return true;
    }

    break;

  // Intentional fallthrow
  case ElementType::Unknown:
  default:

    break;
  }

  // If we get to this point, it meant we never found it
  return false;
}

ConstElementPtr ElementCacheLRU::getElement(const ElementId& eid) const
{
  const long id = eid.getId();
  ConstElementPtr returnPtr;
  switch ( eid.getType().getEnum() )
  {
  case ElementType::Node:
    returnPtr = getNode(id);
    break;

  case ElementType::Way:
    returnPtr = _ways.find(id)->second.first;

    break;

  case ElementType::Relation:
    returnPtr = _relations.find(id)->second.first;

    break;

  // Intentional fallthrow
  case ElementType::Unknown:
  default:

    break;
  }

  return returnPtr;
}

const boost::shared_ptr<const Node> ElementCacheLRU::getNode(long id) const
{
  return _nodes.find(id)->second.first;
}

const boost::shared_ptr<Node> ElementCacheLRU::getNode(long id)
{
  return boost::const_pointer_cast<Node>(_nodes.find(id)->second.first);
}

const boost::shared_ptr<const Relation> ElementCacheLRU::getRelation(long id) const
{
  return _relations.find(id)->second.first;
}

const boost::shared_ptr<Relation> ElementCacheLRU::getRelation(long id)
{
  return boost::const_pointer_cast<Relation>(_relations.find(id)->second.first);
}

const boost::shared_ptr<const Way> ElementCacheLRU::getWay(long id) const
{
  return _ways.find(id)->second.first;
}

const boost::shared_ptr<Way> ElementCacheLRU::getWay(long id)
{
  return boost::const_pointer_cast<Way>(_ways.find(id)->second.first);
}

bool ElementCacheLRU::containsNode(long id) const
{
  return ( _nodes.find(id) != _nodes.end() );
}

bool ElementCacheLRU::containsWay(long id) const
{
  return ( _ways.find(id) != _ways.end() );
}

bool ElementCacheLRU::containsRelation(long id) const
{
  return ( _relations.find(id) != _relations.end() );
}

unsigned long ElementCacheLRU::typeCount(const ElementType::Type typeToCount) const
{
  unsigned long retVal = 0;

  switch ( typeToCount )
  {
  case ElementType::Node:
    retVal = _nodes.size();
    break;

  case ElementType::Way:
    retVal = _ways.size();
    break;

  case ElementType::Relation:
    retVal = _relations.size();
    break;

  default:
    throw HootException("Invalid type passed");
    break;
  }

  return retVal;
}

void ElementCacheLRU::removeElement(const ElementId &eid)
{
  switch ( eid.getType().getEnum() )
  {
  case ElementType::Node:
    _nodes.erase(_nodes.find(eid.getId()));
    break;

  case ElementType::Way:
    _ways.erase(_ways.find(eid.getId()));
    break;

  case ElementType::Relation:
    _relations.erase(_relations.find(eid.getId()));
    break;

  default:
    throw HootException("Invalid type passed");
    break;
  }

  resetElementIterators();
}

void ElementCacheLRU::removeElements(const ElementType::Type type)
{
  switch ( type )
  {
  case ElementType::Node:
    _nodes.erase(_nodes.begin(), _nodes.end());
    break;
  case ElementType::Way:
    _ways.erase(_ways.begin(), _ways.end());
    break;

  case ElementType::Relation:
    _relations.erase(_relations.begin(), _relations.end());
    break;

  default:
    throw HootException("Invalid type passed");
    break;
  }

    resetElementIterators();
}


}
