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
#include "PoiImplicitTagRulesDeriver.h"

// hoot
#include <hoot/core/io/OsmMapReaderFactory.h>
#include <hoot/core/io/PartialOsmMapReader.h>
#include <hoot/core/algorithms/string/StringTokenizer.h>
#include <hoot/core/schema/OsmSchema.h>
#include <hoot/core/elements/Tags.h>
#include <hoot/rnd/io/ImplicitTagRulesWriterFactory.h>
#include <hoot/core/util/StringUtils.h>
#include <hoot/core/util/ConfigOptions.h>
#include <hoot/core/io/ElementVisitorInputStream.h>
#include <hoot/core/visitors/TranslationVisitor.h>

// Tgs
#include <tgs/System/DisableCout.h>

// Qt
#include <QStringBuilder>
#include <QCryptographicHash>

// std
#include <cstdlib>
//#include <clocale>

namespace hoot
{

long PoiImplicitTagRulesDeriver::stxxlMapNodeSize =
  FixedLengthStringToLongMap::node_block_type::raw_size * 5;
long PoiImplicitTagRulesDeriver::stxxlMapLeafSize =
  FixedLengthStringToLongMap::node_block_type::raw_size * 5;

PoiImplicitTagRulesDeriver::PoiImplicitTagRulesDeriver() :
//_wordKvpsToOccuranceCounts(stxxlMapNodeSize, stxxlMapLeafSize),
//_wordCaseMappings(stxxlMapNodeSize, stxxlMapLeafSize),
_avgTagsPerRule(0),
_avgWordsPerRule(0),
_statusUpdateInterval(ConfigOptions().getApidbBulkInserterFileOutputStatusUpdateInterval()),
_highestRuleWordCount(0),
_highestRuleTagCount(0)
{
  //Tgs::DisableCout d;
  //std::setlocale(LC_ALL, "en_US.utf8");
  setenv("STXXLLOGFILE", "/dev/null", 0);
  setenv("STXXLERRLOGFILE", "/dev/null", 0);
}

QStringList PoiImplicitTagRulesDeriver::_getPoiKvps(const Tags& tags) const
{
  LOG_TRACE("Retrieving POI kvps...");

  QStringList poiKvps;
  for (Tags::const_iterator tagItr = tags.begin(); tagItr != tags.end(); ++tagItr)
  {
    const QString kvp = tagItr.key() % "=" % tagItr.value();
    LOG_VART(kvp);
    LOG_VART(OsmSchema::getInstance().getCategories(kvp).intersects(OsmSchemaCategory::poi()));
    if (kvp != QLatin1String("poi=yes") &&
        OsmSchema::getInstance().getCategories(kvp).intersects(OsmSchemaCategory::poi()))
    {
      poiKvps.append(kvp);
    }
  }
  return poiKvps;
}

void PoiImplicitTagRulesDeriver::_updateForNewWord(QString word, const QString kvp)
{
  LOG_TRACE("Updating word: " << word << " with kvp: " << kvp << "...");

  //FixedLengthString fixedLengthWord = _qStrToFixedLengthStr(word);
  const QString lowerCaseWord = word.toLower();
  //FixedLengthString fixedLengthLowerCaseWord = _qStrToFixedLengthStr(lowerCaseWord);
  //if (_wordCaseMappings.find(fixedLengthLowerCaseWord) != _wordCaseMappings.end())
  //if (_wordCaseMappings.contains(fixedLengthLowerCaseWord))
  if (_wordCaseMappings.contains(lowerCaseWord))
  {
    //word = _fixedLengthStrToQStr(_wordCaseMappings[fixedLengthLowerCaseWord]);
    word = _wordCaseMappings[lowerCaseWord];
  }
  else
  {
    //_wordCaseMappings[fixedLengthLowerCaseWord] = fixedLengthWord;
    _wordCaseMappings[lowerCaseWord] = word;
  }

  const QString line = word % QString("\t") % kvp % QString("\n");
  _countFile->write(line.toUtf8());

//  const QString wordKvp = word % ";" % kvp;
//  LOG_VART(wordKvp);
//  FixedLengthString fixedLengthWordKvp = _qStrToFixedLengthStr(wordKvp);
//  if (_wordKvpsToOccuranceCounts.find(fixedLengthWordKvp) == _wordKvpsToOccuranceCounts.end())
//  {
//    _wordKvpsToOccuranceCounts[fixedLengthWordKvp] = 1;
//  }
//  else
//  {
//    _wordKvpsToOccuranceCounts[fixedLengthWordKvp]++;
//  }
//  LOG_VART(_wordKvpsToOccuranceCounts[fixedLengthWordKvp]);

//  const QStringList kvpParts = kvp.split("=");
//  const QString kvpKey = kvpParts[0];
//  const QString kvpVal = kvpParts[1];
//  const QString wordKvpKey = word % ";" % kvpKey;
//  LOG_VART(wordKvpKey);
//  if (!_wordTagKeysToTagValues.contains(wordKvpKey))
//  {
//    QStringList valsList;
//    valsList.append(kvpVal);
//    _wordTagKeysToTagValues[wordKvpKey] = valsList;
//  }
//  else
//  {
//    _wordTagKeysToTagValues[wordKvpKey].append(kvpVal);
//  }
//  LOG_VART(_wordTagKeysToTagValues[wordKvpKey]);
}

bool PoiImplicitTagRulesDeriver::_outputsContainsSqlite(const QStringList outputs)
{
  bool containsSqlite = false;
  for (int i = 0; i < outputs.size(); i++)
  {
    if (outputs.at(i).endsWith(".sqlite"))
    {
      containsSqlite = true;
    }
  }
  return containsSqlite;
}

void PoiImplicitTagRulesDeriver::deriveRules(const QStringList inputs,
                                             const QStringList translationScripts,
                                             const QStringList outputs,
                                             const QStringList typeKeys,
                                             const int minOccurancesThreshold)
{
  if (inputs.isEmpty())
  {
    throw HootException("No inputs were specified.");
  }

  if (outputs.isEmpty())
  {
    throw HootException("No outputs were specified.");
  }
  else if (!_outputsContainsSqlite(outputs))
  {
    throw HootException("Outputs must contain at least on Sqlite database.");
  }

  if (inputs.size() != translationScripts.size())
  {
    LOG_VARD(inputs.size());
    LOG_VARD(translationScripts.size());
    throw HootException(
      "The size of the input datasets list must equal the size of the list of translation scripts.");
  }

  LOG_INFO(
    "Deriving POI implicit tag rules for inputs: " << inputs << ", translation scripts: " <<
    translationScripts << ", type keys: " << typeKeys << ", and minimum occurance threshold: " <<
    minOccurancesThreshold << ".  Writing to outputs: " << outputs << "...");

  _countFile.reset(
    new QTemporaryFile(
      ConfigOptions().getApidbBulkInserterTempFileDir() +
      "/poi-implicit-tag-rules-deriver-temp-XXXXXX"));
  _countFile->setAutoRemove(false); //for debugging only
  if (!_countFile->open())
  {
    throw HootException(QObject::tr("Error opening %1 for writing.").arg(_countFile->fileName()));
  }
  LOG_DEBUG("Opened temp file: " << _countFile->fileName());

  QStringList typeKeysAllowed;
  for (int i = 0; i < typeKeys.size(); i++)
  {
    typeKeysAllowed.append(typeKeys.at(i).toLower());
  }
  LOG_VART(typeKeysAllowed.isEmpty());

  long poiCount = 0;
  long nodeCount = 0;
  for (int i = 0; i < inputs.size(); i++)
  {
    boost::shared_ptr<PartialOsmMapReader> inputReader =
      boost::dynamic_pointer_cast<PartialOsmMapReader>(
        OsmMapReaderFactory::getInstance().createReader(inputs.at(i)));
    inputReader->open(inputs.at(i));
    boost::shared_ptr<ElementInputStream> inputStream =
      boost::dynamic_pointer_cast<ElementInputStream>(inputReader);
    boost::shared_ptr<TranslationVisitor> translationVisitor(new TranslationVisitor());
    translationVisitor->setPath(translationScripts.at(i));
    inputStream.reset(new ElementVisitorInputStream(inputReader, translationVisitor));

    StringTokenizer tokenizer;
    while (inputStream->hasMoreElements())
    {
      ElementPtr element = inputStream->readNextElement();
      if (element->getElementType() == ElementType::Node)
      {
        LOG_VART(element->getTags());
        const QStringList names = element->getTags().getNames();
        LOG_VART(names.size());
        if (names.size() > 0)
        {
          LOG_VART(names);
          Tags relevantTags;
          if (typeKeysAllowed.isEmpty())
          {
            relevantTags = element->getTags();
          }
          else
          {
            for (Tags::const_iterator tagsItr = element->getTags().begin();
                 tagsItr != element->getTags().end(); ++tagsItr)
            {
              const QString tagKey = tagsItr.key();
              if (typeKeysAllowed.contains(tagKey))
              {
                relevantTags.appendValue(tagKey, tagsItr.value());
              }
            }
          }
          LOG_VART(relevantTags);

          const QStringList kvps = _getPoiKvps(relevantTags);
          LOG_VART(kvps.size());
          if (!kvps.isEmpty())
          {
            for (int i = 0; i < names.size(); i++)
            {
              QString name = names.at(i);
              //'=' is used as a map key for kvps, so it needs to be escaped in the word
              if (name.contains("="))
              {
                name = name.replace("=", "%3D");
              }
              for (int j = 0; j < kvps.size(); j++)
              {
                _updateForNewWord(name, kvps.at(j));
              }

              const QStringList nameTokens = tokenizer.tokenize(name);
              LOG_VART(nameTokens.size());
              for (int j = 0; j < nameTokens.size(); j++)
              {
                for (int k = 0; k < kvps.size(); k++)
                {
                  _updateForNewWord(nameTokens.at(j), kvps.at(k));
                }
              }
            }

            poiCount++;

            if (poiCount % _statusUpdateInterval == 0)
            {
              PROGRESS_INFO(
                "Derived implicit tags for " << StringUtils::formatLargeNumber(poiCount) <<
                " POIs.");
            }
          }
        }

        nodeCount++;

        if (nodeCount % (_statusUpdateInterval * 100) == 0)
        {
          PROGRESS_INFO(
            "Parsed " << StringUtils::formatLargeNumber(nodeCount) << " nodes from input.");
        }
      }
    }
    inputReader->finalizePartial();
  }
//  //LOG_VARD(_wordCaseMappings.count());
//  _wordCaseMappings.clear();

//  //TODO: try to reduce these mutiple passes over the data down to a single pass
//  _removeKvpsBelowOccuranceThreshold(minOccurancesThreshold);
//  _removeDuplicatedKeyTypes();
//  LOG_VARD(_wordTagKeysToTagValues.size());
//  _wordTagKeysToTagValues.clear();
//  _generateTagRulesByWord();
//  _rulesByWordToRules(_tagRulesByWord);
//  _unescapeRuleWords();

  _sortTempFileByOccurranceCount();
  _removeDuplicatedKeyTypes();

//  LOG_INFO(
//    "Generated " << StringUtils::formatLargeNumber(_tagRules.size()) <<
//    " implicit tag rules for " << StringUtils::formatLargeNumber(_tagRulesByWord.size()) <<
//    " unique words and " << StringUtils::formatLargeNumber(poiCount) << " POIs (" <<
//    StringUtils::formatLargeNumber(nodeCount) << " nodes parsed).");
//  LOG_INFO("Average words per rule: " << _avgWordsPerRule);
//  LOG_INFO("Average tags per rule: " << _avgTagsPerRule);
//  LOG_INFO("Highest rule word count: " << _highestRuleWordCount);
//  LOG_INFO("Highest rule tag count: " << _highestRuleTagCount);

//  for (int i = 0; i < outputs.size(); i++)
//  {
//    const QString output = outputs.at(i);
//    LOG_INFO("Writing implicit tag rules to " << output << "...");
//    boost::shared_ptr<ImplicitTagRulesWriter> rulesWriter =
//      ImplicitTagRulesWriterFactory::getInstance().createWriter(output);
//    rulesWriter->open(output);
//    rulesWriter->write(_tagRules);
//    rulesWriter->write(_tagRulesByWord);
//    rulesWriter->close();
//  }
}

void PoiImplicitTagRulesDeriver::_removeDuplicatedKeyTypes()
{
  _sortedDedupedCountFile.reset(
    new QTemporaryFile(
      ConfigOptions().getApidbBulkInserterTempFileDir() +
      "/poi-implicit-tag-rules-deriver-temp-XXXXXX"));
  _sortedDedupedCountFile->setAutoRemove(false); //for debugging only
  if (!_sortedDedupedCountFile->open())
  {
    throw HootException(
      QObject::tr("Error opening %1 for writing.").arg(_sortedDedupedCountFile->fileName()));
  }
  LOG_DEBUG("Opened sorted, deduped temp file: " << _sortedDedupedCountFile->fileName());

  const QString line = QString::fromUtf8(_sortedCountFile->readLine().constData());
  const QStringList lineParts = line.split("\t");
  QString word = lineParts[1];
  const QString kvp = lineParts[2];
  const long count = lineParts[0].toLong();
  const QString key = kvp.split("=")[1];
  const QString wordKey = word % ";" % key;

  //The lines are sorted by occurrence count.  So the first time we see one word-key combo, we
  //know it had the highest occurrence count, and we can ignore all subsequent instances since
  //any one feature can't have more than one tag applied to it with the same key.
  if (!_wordKeysToCounts.contains(wordKey))
  {
    _wordKeysToCounts[wordKey] = count;
    //this unescaping must occur during the final temp file write
    if (word.contains("%3D"))
    {
      word = word.replace("%3D", "=");
    }
    else if (word.contains("%3d"))
    {
      word = word.replace("%3d", "=");
    }
    const QString updatedLine = QString::number(count) % "\t" % word % "\t" % kvp % "\n";
    _sortedDedupedCountFile->write(updatedLine.toUtf8());
  }
}

void PoiImplicitTagRulesDeriver::_sortTempFileByOccurranceCount()
{
  _sortedCountFile.reset(
    new QTemporaryFile(
      ConfigOptions().getApidbBulkInserterTempFileDir() +
      "/poi-implicit-tag-rules-deriver-temp-XXXXXX"));
  _sortedCountFile->setAutoRemove(false); //for debugging only
  if (!_sortedCountFile->open())
  {
    throw HootException(
      QObject::tr("Error opening %1 for writing.").arg(_sortedCountFile->fileName()));
  }
  LOG_DEBUG("Opened sorted temp file: " << _sortedCountFile->fileName());

  const QString cmd =
    "sort " + _countFile->fileName() + " | uniq -c | sort -n -r -o " + _sortedCountFile->fileName();
  if (std::system(cmd.toStdString().c_str()) != 0)
  {
    throw HootException("Unable to sort input file.");
  }
}

//void PoiImplicitTagRulesDeriver::_removeKvpsBelowOccuranceThreshold(const int minOccurancesThreshold)
//{
//  if (minOccurancesThreshold == 1)
//  {
//    return;
//  }

//  LOG_DEBUG("Removing tags below mininum occurance threshold: " << minOccurancesThreshold << "...");

//  Tgs::DisableCout d;
//  FixedLengthStringToLongMap updatedCounts(stxxlMapNodeSize, stxxlMapLeafSize);
//  QMap<QString, QStringList> updatedValues; //*

//  long kvpRemovalCount = 0;
//  for (FixedLengthStringToLongMap::const_iterator kvpCountsItr = _wordKvpsToOccuranceCounts.begin();
//       kvpCountsItr != _wordKvpsToOccuranceCounts.end(); ++kvpCountsItr)
//  {
//    const long count = kvpCountsItr->second;
//    LOG_VART(count);
//    if (count >= minOccurancesThreshold)
//    {
//      const QString wordKvp = _fixedLengthStrToQStr(kvpCountsItr->first);
//      LOG_VART(wordKvp);
//      const QStringList keyParts = wordKvp.split(";");
//      const QString word = keyParts[0];
//      //LOG_VART(word);
//      const QString kvp = keyParts[1];
//      //LOG_VART(kvp);
//      const QStringList kvpParts = kvp.split("=");
//      const QString kvpKey = kvpParts[0];
//      const QString kvpVal = kvpParts[1];
//      const QString wordKvpKey = word % ";" % kvpKey;

//      if (!wordKvp.contains(";"))
//      {
//        LOG_VARE(wordKvp);
//      }
//      FixedLengthString fixedLengthWordKvp = _qStrToFixedLengthStr(wordKvp);
//      LOG_VART(fixedLengthWordKvp.data);
//      updatedCounts[fixedLengthWordKvp] = count;
//      if (!updatedValues.contains(wordKvpKey))
//      {
//        updatedValues[wordKvpKey] = QStringList();
//      }
//      updatedValues[wordKvpKey].append(kvpVal);
//      LOG_VART(updatedValues[wordKvpKey]);
//    }
//    else
//    {
//      kvpRemovalCount++;
//    }
//  }

//  _wordKvpsToOccuranceCounts.clear();
//  _wordKvpsToOccuranceCounts.insert(updatedCounts.begin(), updatedCounts.end());
//  _wordTagKeysToTagValues = updatedValues;

//  LOG_DEBUG(
//    "Removed " << StringUtils::formatLargeNumber(kvpRemovalCount) << " tags whose " <<
//    "occurrance count fell below the minimum occurrance threshold of " << minOccurancesThreshold);
//}

//void PoiImplicitTagRulesDeriver::_removeDuplicatedKeyTypes()
//{
//  LOG_DEBUG("Removing duplicated tag types...");

//  Tgs::DisableCout d;
//  FixedLengthStringToLongMap updatedCounts(stxxlMapNodeSize, stxxlMapLeafSize);
//  QMap<QString, QStringList> updatedValues; //*

//  long duplicatedKeyTypeRemovalCount = 0;
//  for (QMap<QString, QStringList>::const_iterator valsItr = _wordTagKeysToTagValues.begin();
//       valsItr != _wordTagKeysToTagValues.end(); ++valsItr)
//  {
//    const QString wordKvpKey = valsItr.key();
//    LOG_VART(wordKvpKey);
//    const QStringList vals = valsItr.value();
//    LOG_VART(vals.size());

//    assert(vals.size() != 0);
//    if (vals.size() > 1)
//    {
//      LOG_TRACE(vals.size() << " values mapped to wordKvpKey: " << wordKvpKey);

//      QString highestOccurranceWordKvp;
//      long highestOccurranceCount = 0;

//      for (int i = 0; i < vals.size(); i++)
//      {
//        const QString wordKvp = wordKvpKey % "=" % vals.at(i);
//        LOG_VART(wordKvp);
//        FixedLengthString fixedLengthWordKvp = _qStrToFixedLengthStr(wordKvp);
//        const long occurranceCount = _wordKvpsToOccuranceCounts[fixedLengthWordKvp];
//        LOG_VART(occurranceCount);
//        if (occurranceCount > highestOccurranceCount)
//        {
//          highestOccurranceCount = occurranceCount;
//          LOG_VART(highestOccurranceCount);
//          highestOccurranceWordKvp = wordKvp;
//          LOG_VART(highestOccurranceWordKvp);
//        }
//      }

//      if (highestOccurranceCount > 0)
//      {
//        if (!highestOccurranceWordKvp.contains(";"))
//        {
//          LOG_VARE(highestOccurranceWordKvp);
//        }
//        FixedLengthString fixedLengthHighestOccurranceWordKvp =
//          _qStrToFixedLengthStr(highestOccurranceWordKvp);
//        updatedCounts[fixedLengthHighestOccurranceWordKvp] = highestOccurranceCount;
//        LOG_VART(updatedCounts[fixedLengthHighestOccurranceWordKvp]);
//        const QString highestOccurranceVal = highestOccurranceWordKvp.split("=")[1];
//        if (!updatedValues.contains(wordKvpKey))
//        {
//          updatedValues[wordKvpKey] = QStringList();
//        }
//        updatedValues[wordKvpKey].append(highestOccurranceVal);
//        LOG_VART(updatedValues[wordKvpKey]);
//      }

//      //removed all but one of the tag values for the same tag key
//      duplicatedKeyTypeRemovalCount += vals.size() - 1;
//    }
//    else //size == 1 - tag key has only one tag value associated with it, so no removal necessary
//    {
//      LOG_TRACE("One value mapped to wordKvpKey: " << wordKvpKey);

//      const QString wordKvp = wordKvpKey % "=" % vals.at(0);
//      if (!wordKvp.contains(";"))
//      {
//        LOG_VARE(wordKvp);
//      }
//      FixedLengthString fixedLengthWordKvp = _qStrToFixedLengthStr(wordKvp);
//      updatedCounts[fixedLengthWordKvp] = _wordKvpsToOccuranceCounts[fixedLengthWordKvp];
//      LOG_VART(updatedCounts[fixedLengthWordKvp]);
//      updatedValues[wordKvpKey] = _wordTagKeysToTagValues[wordKvpKey];
//      LOG_VART(updatedValues[wordKvpKey]);
//    }
//  }

//  _wordKvpsToOccuranceCounts.clear();
//  _wordKvpsToOccuranceCounts.insert(updatedCounts.begin(), updatedCounts.end());
//  _wordTagKeysToTagValues= updatedValues;

//  LOG_DEBUG(
//    "Removed " << StringUtils::formatLargeNumber(duplicatedKeyTypeRemovalCount) <<
//    " tag values belonged to the same tag key for a given word.");
//}

//void PoiImplicitTagRulesDeriver::_generateTagRulesByWord()
//{
//  LOG_DEBUG("Generating rules by word output...");

//  //_tagRulesByWord: key=<word>, value=map: key=<kvp>, value=<kvp occurance count>

//  for (FixedLengthStringToLongMap::const_iterator kvpsWithCountsItr = _wordKvpsToOccuranceCounts.begin();
//       kvpsWithCountsItr != _wordKvpsToOccuranceCounts.end(); ++kvpsWithCountsItr)
//  {
//    const QString wordKvp = _fixedLengthStrToQStr(kvpsWithCountsItr->first);
//    LOG_VART(wordKvp);
//    const QStringList wordKvpParts = wordKvp.split(";");
//    QString word = wordKvpParts[0];
//    if (word.contains("="))
//    {
//      LOG_VARE(word);
//    }
//    LOG_VART(word);

//    const QString kvp = wordKvpParts[1];
//    LOG_VART(kvp);
//    const long kvpCount = kvpsWithCountsItr->second;
//    LOG_VART(kvpCount);

//    if (!_tagRulesByWord.contains(word))
//    {
//      _tagRulesByWord[word] = QMap<QString, long>();
//    }
//    QMap<QString, long> kvpsWithCounts = _tagRulesByWord[word]; //*
//    kvpsWithCounts[kvp] = kvpCount;
//    _tagRulesByWord[word] = kvpsWithCounts;
//  }
//  LOG_VARD(_wordKvpsToOccuranceCounts.size());
//  _wordKvpsToOccuranceCounts.clear();
//}

//void PoiImplicitTagRulesDeriver::_rulesByWordToRules(const ImplicitTagRulesByWord& rulesByWord)
//{
//  LOG_DEBUG("Generating rules output...");

//  //key=<concatenated kvps list>, value=<rule>
//  QMap<QString, ImplicitTagRulePtr> tagsToRules;
//  //key=<word>, value=map: key=<kvp>, value=<kvp occurance count>
//  for (ImplicitTagRulesByWord::const_iterator rulesByWordItr = rulesByWord.begin();
//       rulesByWordItr != rulesByWord.end(); ++rulesByWordItr)
//  {
//    QString word = rulesByWordItr.key();
//    if (word.contains("="))
//    {
//      LOG_VARE(word);
//    }
//    LOG_VART(word);

//    const QSet<QString> kvps = rulesByWordItr.value().keys().toSet();
//    const QString kvpsStr = _kvpsToString(kvps);
//    LOG_VART(kvpsStr);

//    ImplicitTagRulePtr rule;
//    if (tagsToRules.contains(kvpsStr))
//    {
//      LOG_TRACE("Tag set already exists for rule.");
//      rule = tagsToRules[kvpsStr];
//    }
//    else
//    {
//      LOG_TRACE("Creating new rule for tag set...");
//      rule.reset(new ImplicitTagRule());
//      tagsToRules[kvpsStr] = rule;
//      _tagRules.append(rule);
//      LOG_VART(_tagRules.size());
//      const Tags tags = _kvpsToTags(kvps);
//      LOG_VART(tags);
//      rule->setTags(tags);
//    }
//    rule->getWords().insert(word);

//    LOG_VART(rule->getWords());
//    LOG_VART(rule->getTags());
//  }

//  long totalWordInstances = 0;
//  long totalTagInstances = 0;
//  for (ImplicitTagRules::const_iterator rulesItr = _tagRules.begin(); rulesItr != _tagRules.end();
//       ++rulesItr)
//  {
//    const ImplicitTagRulePtr rule = *rulesItr;

//    const long ruleWordCount = rule->getWords().size();
//    totalWordInstances += ruleWordCount;
//    if (ruleWordCount > _highestRuleWordCount)
//    {
//      _highestRuleWordCount = ruleWordCount;
//    }

//    const long ruleTagCount = rule->getTags().size();
//    totalTagInstances += ruleTagCount;
//    if (ruleTagCount > _highestRuleTagCount)
//    {
//      _highestRuleTagCount = ruleTagCount;
//    }
//  }
//  if (_tagRules.size() > 0)
//  {
//    _avgWordsPerRule = totalWordInstances / _tagRules.size();
//    _avgTagsPerRule = totalTagInstances / _tagRules.size();
//  }
//}

//Tags PoiImplicitTagRulesDeriver::_kvpsToTags(const QSet<QString>& kvps)
//{
//  Tags tags;
//  for (QSet<QString>::const_iterator kvpsItr = kvps.begin(); kvpsItr != kvps.end(); ++kvpsItr)
//  {
//    tags.appendValue(*kvpsItr);
//  }
//  return tags;
//}

//QString PoiImplicitTagRulesDeriver::_kvpsToString(const QSet<QString>& kvps)
//{
//  QString kvpsStr;
//  for (QSet<QString>::const_iterator kvpsItr = kvps.begin(); kvpsItr != kvps.end(); ++kvpsItr)
//  {
//    kvpsStr += *kvpsItr % ";";
//  }
//  kvpsStr.chop(1);
//  return kvpsStr;
//}

//void PoiImplicitTagRulesDeriver::_unescapeRuleWords()
//{
//  LOG_DEBUG("Unescaping rule words...");

//  ImplicitTagRulesByWord rulesByWord;
//  for (ImplicitTagRulesByWord::const_iterator rulesByWordItr = _tagRulesByWord.begin();
//       rulesByWordItr != _tagRulesByWord.end(); ++rulesByWordItr)
//  {
//    QString word = rulesByWordItr.key();
//    if (word.contains("="))
//    {
//      LOG_VARE(word);
//    }
//    if (word.contains("%3D"))
//    {
//      word = word.replace("%3D", "=");
//    }
//    else if (word.contains("%3d"))
//    {
//      word = word.replace("%3d", "=");
//    }

//    rulesByWord[word] = rulesByWordItr.value();
//  }
//  _tagRulesByWord = rulesByWord;

//  for (ImplicitTagRules::iterator rulesItr = _tagRules.begin(); rulesItr != _tagRules.end();
//       ++rulesItr)
//  {
//    ImplicitTagRulePtr rule = *rulesItr;
//    const QSet<QString> ruleWords = rule->getWords();
//    QSet<QString> modifiedRuleWords;
//    for (QSet<QString>::const_iterator wordsItr = ruleWords.begin(); wordsItr != ruleWords.end();
//         ++wordsItr)
//    {
//      QString word = *wordsItr;
//      if (word.contains("%3D"))
//      {
//        word = word.replace("%3D", "=");
//      }
//      else if (word.contains("%3d"))
//      {
//        word = word.replace("%3d", "=");
//      }
//      modifiedRuleWords.insert(word);
//    }
//    rule->setWords(modifiedRuleWords);
//  }
//}

FixedLengthString PoiImplicitTagRulesDeriver::_qStrToFixedLengthStr(const QString wordKvp)
{
  FixedLengthString fixedLengthWordKvp;
  memset(fixedLengthWordKvp.data, 0, sizeof fixedLengthWordKvp.data);
  std::wcstombs(fixedLengthWordKvp.data, wordKvp.toStdWString().c_str(), MAX_KEY_LEN);
  return fixedLengthWordKvp;
}

//FixedLengthString PoiImplicitTagRulesDeriver::_qStrToFixedLengthStr2(const QString wordKvp)
//{
//  FixedLengthString fixedLengthWordKvp;
//  memset(fixedLengthWordKvp.data, 0, sizeof fixedLengthWordKvp.data);
//  QCryptographicHash hash(QCryptographicHash::Sha1);
//  hash.addData(wordKvp.toAscii().constData());
//  const QString hashStr = QString::fromUtf8(hash.result().toHex());;
//  std::memcpy(fixedLengthWordKvp.data, hashStr.toStdString().c_str(), hashStr.size());
//  return fixedLengthWordKvp;
//}

QString PoiImplicitTagRulesDeriver::_fixedLengthStrToQStr(const FixedLengthString& fixedLengthStr)
{
  wchar_t wKey[MAX_KEY_LEN];
  std::mbstowcs(wKey, fixedLengthStr.data, MAX_KEY_LEN);
  return QString::fromWCharArray(wKey);
}

//QString PoiImplicitTagRulesDeriver::_fixedLengthStrToQStr2(const FixedLengthString& fixedLengthStr)
//{
//  const QString hashStr = QString::fromUtf8(fixedLengthStr.data);
//  QByteArray bytes;
//  bytes.append(hashStr);
//  return QByteArray::fromBase64(bytes).data();
//}

//QMap<QString, long> PoiImplicitTagRulesDeriver::_stxxlMapToQtMap(
//  const FixedLengthStringToLongMap& stxxlMap)
//{
//  LOG_DEBUG("Converting stxxl map to qt map...");
//  QMap<QString, long> qtMap;
//  for (FixedLengthStringToLongMap::const_iterator mapItr = stxxlMap.begin();
//       mapItr != stxxlMap.end(); ++mapItr)
//  {
//    const QString key = _fixedLengthStrToQStr(mapItr->first);
//    LOG_VART(key);
//    const long value = mapItr->second;
//    LOG_VART(value);
//    qtMap[key] = value;
//  }
//  return qtMap;
//}

}
