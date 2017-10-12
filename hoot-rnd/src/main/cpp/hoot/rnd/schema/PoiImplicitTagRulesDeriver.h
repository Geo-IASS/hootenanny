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
#ifndef POIIMPLICITTAGRULESDERIVER_H
#define POIIMPLICITTAGRULESDERIVER_H

// Hoot
#include <hoot/rnd/schema/ImplicitTagRule.h>
#include <hoot/rnd/util/FixedLengthString.h>

// Qt
#include <QString>
#include <QMap>

namespace hoot
{

class Tags;

/**
 * Derives implicit tag rules for POIs and writes the rules to various output formats
 */
class PoiImplicitTagRulesDeriver
{

public:

  static long stxxlMapNodeSize;
  static long stxxlMapLeafSize;

  PoiImplicitTagRulesDeriver();

  /**
   * Derives implicit tag rules for POIs given input name data and writes the rules to output
   *
   * @param inputs a list of hoot supported feature input formats to derive rules from
   * @param translationScripts list of OSM translation scripts corresponding to the datasets
   * specified by the inputs parameter
   * @param outputs a list of hoot supported implicit tag rule output formats
   * @param typeKeys an optional list of OSM tag keys for which to derive rules; if empty, rules
   * will be derived for all OSM types
   * @param minOccurancesThreshold an optional minimum tag occurrance threshold to use when
   * deriving rules; rules will only be derived when a word is associated with a particular tag
   * at least as many times as this value
   */
  void deriveRules(const QStringList inputs, const QStringList translationScripts,
                   const QStringList outputs, const QStringList typeKeys = QStringList(),
                   const int minOccurancesThreshold = 1);

private:

  //for testing
  friend class PoiImplicitTagRulesDeriverTest;

  //key=<word>;<kvp>, value=<kvp occurance count>
  FixedLengthStringToLongMap _wordKvpsToOccuranceCounts;
  //key=<word>;<tag key>, value=<tag values>
  QMap<QString, QStringList> _wordTagKeysToTagValues; //TODO: replace with stxxl map
  //key=<lower case word>, value=<word>
  QMap<QString, QString> _wordCaseMappings; //* //TODO: replace with stxxl map
  //TODO
  //QStringList _wordsToIgnore;
  double _avgTagsPerRule;
  double _avgWordsPerRule;
  long _statusUpdateInterval;
  long _highestRuleWordCount;
  long _highestRuleTagCount;

  ImplicitTagRulesByWord _tagRulesByWord;
  ImplicitTagRules _tagRules;

  void _updateForNewWord(QString word, const QString kvp);
  QStringList _getPoiKvps(const Tags& tags) const;
  void _removeKvpsBelowOccuranceThreshold(const int minOccurancesThreshold);
  void _removeDuplicatedKeyTypes();
  void _removeIrrelevantKeyTypes(const QStringList typeKeysAllowed);
  void _generateTagRulesByWord();
  void _rulesByWordToRules(const ImplicitTagRulesByWord& rulesByWord);
  Tags _kvpsToTags(const QSet<QString>& kvps);
  QString _kvpsToString(const QSet<QString>& kvps);
  void _unescapeRuleWords();
  //temp
  QMap<QString, long> _stxxlMapToQtMap(const FixedLengthStringToLongMap& stxxlMap);
  FixedLengthString _qStrToFixedLengthStr(const QString wordKvp);
  QString _fixedLengthStrToQStr(const FixedLengthString& fixedLengthStr);
};

}

#endif // POIIMPLICITTAGRULESDERIVER_H
