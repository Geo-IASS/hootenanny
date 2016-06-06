#ifndef NETWORKMERGER_H
#define NETWORKMERGER_H

// hoot
#include <hoot/core/algorithms/linearreference/WayMatchStringMapping.h>
#include <hoot/core/conflate/MergerBase.h>

#include "EdgeMatch.h"
#include "NetworkDetails.h"

namespace hoot
{

/**
 * Merges network pairs.
 *
 * In the case of network matches we're guaranteed there is no overlap between matches so we can
 * use some of the functions in HighwaySnapMerger, but others are too complex/imprecise.
 */
class NetworkMerger : public MergerBase
{
public:
  /**
   * Constructed with a set of element matching pairs. The pairs are generally Unknown1 as first
   * and Unknown2 as second.
   */
  NetworkMerger(const set< pair<ElementId, ElementId> >& pairs, ConstEdgeMatchPtr edgeMatch,
    ConstNetworkDetailsPtr details);

  virtual void apply(const OsmMapPtr& map, vector< pair<ElementId, ElementId> >& replaced) const;

  virtual QString toString() const;

protected:
  virtual PairsSet& getPairs() { return _pairs; }
  virtual const PairsSet& getPairs() const { return _pairs; }

private:
  set< pair<ElementId, ElementId> > _pairs;
  ConstEdgeMatchPtr _edgeMatch;
  ConstNetworkDetailsPtr _details;

  WaySublineMatchStringPtr _createMatchString() const { return WaySublineMatchStringPtr(); }
};

}

#endif // NETWORKMERGER_H
