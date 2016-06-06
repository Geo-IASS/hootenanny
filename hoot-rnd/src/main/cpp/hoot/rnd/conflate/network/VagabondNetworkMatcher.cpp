#include "VagabondNetworkMatcher.h"

#include <hoot/core/conflate/polygon/extractors/AngleHistogramExtractor.h>
#include <hoot/core/conflate/polygon/extractors/HausdorffDistanceExtractor.h>

#include "EdgeMatchSetFinder.h"

namespace hoot
{

VagabondNetworkMatcher::VagabondNetworkMatcher() :
  _dampen(0.8)
{
  _pr.reset(new IndexedEdgeMatchSet());
}

shared_ptr<VagabondNetworkMatcher> VagabondNetworkMatcher::create()
{
  return shared_ptr<VagabondNetworkMatcher>(new VagabondNetworkMatcher());
}

QList<NetworkEdgeScorePtr> VagabondNetworkMatcher::getAllEdgeScores() const
{
  QList<NetworkEdgeScorePtr> result;

  for (IndexedEdgeMatchSet::MatchHash::const_iterator it = _pr->getAllMatches().begin();
    it != _pr->getAllMatches().end(); ++it)
  {
    NetworkEdgeScorePtr p(new NetworkEdgeScore(it.key(),
      it.value() * _pr->getAllMatches().size(), 1));
    p->setUid(it.key()->toString());

    result.append(p);
  }

  return result;
}

QList<NetworkVertexScorePtr> VagabondNetworkMatcher::getAllVertexScores() const
{
  QList<NetworkVertexScorePtr> result;
  return result;
}

void VagabondNetworkMatcher::iterate()
{
  iteratePageRank();
}

void VagabondNetworkMatcher::iteratePageRank()
{
  // assign a new PR to all edge pairs based on
  // PR(A) = (1 - _damping) / N + d * (PR(B) / L(B) + PR(C) / L(C) + ...)
  // where A is the edge being traversed into
  // B, C, ... are edges that traverse into A and L is the number of outbound edges for that edge
  // ###
  // ### And then a bit of experimentation that isn't documented. Read the code.
  // ###

  IndexedEdgeMatchSetPtr newHash = _pr->clone();

  double sum = 0.0;

  for (IndexedEdgeMatchSet::MatchHash::iterator it = newHash->getAllMatches().begin();
    it != newHash->getAllMatches().end(); ++it)
  {
    sum += it.value();
    it.value() = 0;
  }

  LOG_VAR(sum);
  LOG_VAR(_links.size());

  // add this weight to all edges at the end.
  double allWeight = (1 - _dampen) / _pr->getSize();

  /// @todo this is much less efficient then some iterator approaches, but it is easy to write &
  /// read. if this works and it is a bottle neck, please fix it.
  for (IndexedEdgeMatchSet::MatchHash::iterator it = newHash->getAllMatches().begin();
    it != newHash->getAllMatches().end(); ++it)
  {
    EdgeMatchPtr from = it.key();
    QList<EdgeMatchPtr> values = _links.values(from);

    if (values.size() != 0)
    {
      double contribution = (_pr->getScore(from) / (values.size())) * _dampen;

      //LOG_VAR(from);
      foreach (EdgeMatchPtr to, values)
      {
        newHash->setScore(to, newHash->getScore(to) + contribution);
//        LOG_VAR(to);
//        LOG_VAR(newHash[to]);
//        LOG_VAR(contribution);
      }
    }
    else
    {
      allWeight += _pr->getScore(from) / (double)_pr->getSize();
    }
  }
  LOG_VAR(allWeight);

  for (IndexedEdgeMatchSet::MatchHash::iterator it = newHash->getAllMatches().begin();
    it != newHash->getAllMatches().end(); ++it)
  {
    it.value() = it.value() + allWeight;
  }

  _pr = newHash;
}

void VagabondNetworkMatcher::iterateVoting()
{
  // assign a new PR to all edge pairs based on
  // PR(A) = (1 - _damping) / N + d * (PR(B) / L(B) + PR(C) / L(C) + ...)
  // where A is the edge being traversed into
  // B, C, ... are edges that traverse into A and L is the number of outbound edges for that edge

  IndexedEdgeMatchSetPtr newHash = _pr->clone();

  double sum = 0.0;

  for (IndexedEdgeMatchSet::MatchHash::iterator it = newHash->getAllMatches().begin();
    it != newHash->getAllMatches().end(); ++it)
  {
    sum += it.value();
    it.value() = 0;
  }

  LOG_VAR(sum);
  LOG_VAR(_links.size());

  // add this weight to all edges at the end.
  double allWeight = (1 - _dampen) / _pr->getSize();

  // this is much less efficient then some iterator approaches, but it is easy to write & read.
  // if this works and it is a bottle neck, please fix it.
  for (IndexedEdgeMatchSet::MatchHash::iterator it = newHash->getAllMatches().begin();
    it != newHash->getAllMatches().end(); ++it)
  {
    EdgeMatchPtr from = it.key();
    QList<EdgeMatchPtr> values = _links.values(from);

    if (values.size() != 0)
    {
      double contribution = _pr->getScore(from) * _dampen;

      LOG_VAR(from);
      foreach (EdgeMatchPtr to, values)
      {
        newHash->setScore(to, newHash->getScore(to) + contribution);
        LOG_VAR(to);
        LOG_VAR(newHash->getScore(to));
        LOG_VAR(contribution);
      }
    }
  }
  LOG_VAR(allWeight);

  double finalSum = 0.0;
  for (IndexedEdgeMatchSet::MatchHash::iterator it = newHash->getAllMatches().begin();
    it != newHash->getAllMatches().end(); ++it)
  {
    it.value() = it.value() + allWeight;
    finalSum += it.value();
  }

  for (IndexedEdgeMatchSet::MatchHash::iterator it = newHash->getAllMatches().begin();
    it != newHash->getAllMatches().end(); ++it)
  {
    it.value() /= finalSum;
  }

  _pr = newHash;
}

void VagabondNetworkMatcher::matchNetworks(ConstOsmMapPtr map, OsmNetworkPtr n1, OsmNetworkPtr n2)
{
  _map = map;
  _n1 = n1;
  _n2 = n2;
  _details1.reset(new NetworkDetails(map, n1));
  _details2.reset(new NetworkDetails(map, n2));

  // calculate all the candidate edge pairs.
  LOG_INFO("Calculating edge index...");
  _createEdge2Index();
  LOG_INFO("Calculating edge matches...");
  _calculateEdgeMatches();

  // calculate the edge pairs that can be accessed from each candidate edge pair
  LOG_INFO("Calculating edge links...");
  _calculateEdgeLinks();

  // for each link distribute a PR based on length and frequency an edge is used
  LOG_INFO("Distributing PR...");
  _distributePrLengthWeighted();
  // distribute an even PR to all edge pairs
  //_distributePrEvenly();
}

void VagabondNetworkMatcher::_calculateEdgeLinks()
{
  for (QHash<EdgeMatchPtr, double>::const_iterator it = _pr->getAllMatches().begin();
    it != _pr->getAllMatches().end(); ++it)
  {
    EdgeMatchPtr em = it.key();
    ConstNetworkVertexPtr from1, from2, to1, to2;
    from1 = em->getString1()->getFrom();
    to1 = em->getString1()->getTo();
    from2 = em->getString2()->getFrom();
    to2 = em->getString2()->getTo();

    // get all the edges that connect to from1/from2 and are also a proper pair.
    QSet<EdgeMatchPtr> fromLinks = _getConnectedEdges(from1, from2);

    // get all the edges that connect to to1/to2 and are also a proper pair.
    QSet<EdgeMatchPtr> toLinks = _getConnectedEdges(to1, to2);

    QSet<EdgeMatchPtr> links = fromLinks | toLinks;

    foreach(EdgeMatchPtr other, links)
    {
      // if the other edge isn't part of this edge.
      if (other->overlaps(em) == false)
      {
        _links.insertMulti(em, other);
      }
    }
  }

  LOG_VAR(_links.size());
}

void VagabondNetworkMatcher::_calculateEdgeMatches()
{
  EdgeMatchSetFinder finder(_details1, _pr, _n1, _n2);

  // go through all the n1 edges
  const OsmNetwork::EdgeMap& em = _n1->getEdgeMap();
  int count = 0;
  //LOG_VAR(em);
  for (OsmNetwork::EdgeMap::const_iterator it = em.begin(); it != em.end(); ++it)
  {
    NetworkEdgePtr e1 = it.value();
    // find all the n2 edges that are in range of this one
    Envelope env = _details1->getEnvelope(it.value());
    env.expandBy(_details1->getSearchRadius(it.value()));
    IntersectionIterator iit = _createIterator(env, _edge2Index);
    //LOG_VAR(e1);

    while (iit.next())
    {
      NetworkEdgePtr e2 = _index2Edge[iit.getId()];

      if (_details1->getPartialEdgeMatchScore(e1, e2) > 0)
      {
        // add all the EdgeMatches that are seeded with this edge pair.
        finder.addEdgeMatches(e1, e2);
      }
    }
    if (Log::getInstance().getLevel() <= Log::Info)
    {
      cout << "Calculating edge matches: " << count++ << " / " << em.size() << "\t\r" << std::flush;
    }
  }
  if (Log::getInstance().getLevel() <= Log::Info)
  {
    cout << endl;
  }

  LOG_VAR(_pr);
}

double VagabondNetworkMatcher::_calculateLinkWeight(QHash<ConstNetworkEdgePtr, int>& counts,
  ConstEdgeStringPtr str)
{
  double result = 0.0;
  for (int i = 0; i < str->getAllEdges().size(); ++i)
  {
    ConstNetworkEdgePtr e = str->getAllEdges()[i].e;
    double w = 1.0 / counts[e];
    double l = _details1->calculateLength(e);
    result += w * l;
  }

  return result;
}


void VagabondNetworkMatcher::_countEdgesUsed(QHash<ConstNetworkEdgePtr, int>& counts,
  ConstEdgeStringPtr str)
{
  for (int i = 0; i < str->getAllEdges().size(); ++i)
  {
    counts[str->getAllEdges()[i].e]++;
  }
}

void VagabondNetworkMatcher::_distributePrLengthWeighted()
{
  QHash<ConstNetworkEdgePtr, int> counts;

  // go through all the links
  for (IndexedEdgeMatchSet::MatchHash::iterator it = _pr->getAllMatches().begin();
    it != _pr->getAllMatches().end(); ++it)
  {
    // go through each edge and count the number of times it is used.
    _countEdgesUsed(counts, it.key()->getString1());
    _countEdgesUsed(counts, it.key()->getString2());
  }

  // distribute a weight equal to sum of v of all edges in a link.
  // where v is the length * (1 / number of times an edge is referenced).
  double sum = 0.0;
  for (IndexedEdgeMatchSet::MatchHash::iterator it = _pr->getAllMatches().begin();
    it != _pr->getAllMatches().end(); ++it)
  {
    double w = 0.0;
    w += _calculateLinkWeight(counts, it.key()->getString1());
    w += _calculateLinkWeight(counts, it.key()->getString2());
    it.value() = w;
    sum += w;
  }

  // normalize the values so they add to 1.
  for (IndexedEdgeMatchSet::MatchHash::iterator it = _pr->getAllMatches().begin();
    it != _pr->getAllMatches().end(); ++it)
  {
    it.value() = it.value() / sum;
  }
}

void VagabondNetworkMatcher::_distributePrEvenly()
{
  // distribute an even PR to all edge pairs
  double startPr = 1.0 / (double)_pr->getSize();
  LOG_VAR(startPr);
  for (IndexedEdgeMatchSet::MatchHash::iterator it = _pr->getAllMatches().begin();
    it != _pr->getAllMatches().end(); ++it)
  {
    it.value() = startPr;
  }
}

QSet<EdgeMatchPtr> VagabondNetworkMatcher::_getConnectedEdges(ConstNetworkVertexPtr v1,
  ConstNetworkVertexPtr v2)
{
  return _pr->getMatchesWithTermination(v1, v2);
}

}
