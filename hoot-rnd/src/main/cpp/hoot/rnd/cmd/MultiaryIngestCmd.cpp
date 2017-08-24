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

// Hoot
#include <hoot/core/util/Factory.h>
#include <hoot/core/cmd/BaseCommand.h>
#include <hoot/core/io/ChangesetDeriver.h>
#include <hoot/core/io/ElementSorter.h>
#include <hoot/core/io/OsmMapReaderFactory.h>
#include <hoot/core/io/OsmMapWriterFactory.h>
#include <hoot/core/io/HootApiDbWriter.h>
#include <hoot/rnd/io/SparkChangesetWriter.h>
#include <hoot/core/io/HootApiDbReader.h>
#include <hoot/core/io/OsmChangeWriterFactory.h>
#include <hoot/rnd/io/ElementCriterionVisitorInputStream.h>
#include <hoot/core/filters/PoiCriterion.h>
#include <hoot/core/visitors/TranslationVisitor.h>
#include <hoot/core/io/ElementOutputStream.h>
#include <hoot/core/io/GeoNamesReader.h>
#include <hoot/core/visitors/CalculateHashVisitor2.h>
#include <hoot/core/util/FileUtils.h>

// Qt
#include <QUuid>
#include <QElapsedTimer>

using namespace std;

namespace hoot
{

/**
 * This command ingests data into the Multiary POI conflation workflow.  It takes a supported
 * data input, a reference database layer, and a changeset output location.  The input is
 * filtered down to POIs only and translated to OSM, then sorted by element ID (if not already).
 * Finally, the input data is then compared to the database ref layer in order to derive the
 * difference between the two in the form of a changeset.  The changeset changes are written both
 * to the reference layer as features and to a changeset output file as change statements for later
 * use by Spark.  Alternatively, if the specified database reference layer is empty, no changeset
 * is derived and the entire contents of the data input are simply written directly to the
 * reference layer.
 *
 * This command requires that the input be a streamable format, the output layer be a Hootenanny
 * API database layer, and the changeset output format be a Spark changeset.
 */
class MultiaryIngestCmd : public BaseCommand
{
public:

  static string className() { return "hoot::MultiaryIngestCmd"; }

  MultiaryIngestCmd() :
  _sortInput(false),
  _isGeoNamesInput(false),
  _changesParsed(0)
  {
  }

  virtual ~MultiaryIngestCmd()
  {
    if (_sortInput)
    {
      //delete the temporary db layer used for sorting
      LOG_DEBUG("Deleting temporary map: " << _sortedNewInput << "...");
      HootApiDbWriter().deleteMap(_sortedNewInput);
    }
  }

  virtual QString getName() const { return "multiary-ingest"; }

  virtual int runSimple(QStringList args)
  {
    if (args.size() != 3 && args.size() != 4)
    {
      cout << getHelp() << endl << endl;
      throw HootException(QString("%1 takes three or four parameters.").arg(getName()));
    }

    const QString newInput = args[0];
    _isGeoNamesInput = GeoNamesReader().isSupported(newInput);
    const QString referenceOutput = args[1];
    const QString changesetOutput = args[2];
    _sortInput = true;
    if (args.size() == 4 && args[3].toLower().trimmed() == "false")
    {
      _sortInput = false;
    }

    LOG_VARD(newInput);
    LOG_VARD(referenceOutput);
    LOG_VARD(changesetOutput);
    LOG_VARD(_sortInput);

    if (!OsmMapReaderFactory::getInstance().hasElementInputStream(newInput))
    {
      throw IllegalArgumentException(
        QString("This command does not support non-streamable inputs.  See README.md - ") +
        QString("Supported Data Formats for more details.  Input: ") + newInput);
    }

    if (!HootApiDbReader().isSupported(referenceOutput))
    {
      throw HootException(
        getName() + " only supports a hootapidb:// data source as the reference output.  " +
        "Specified reference layer: " + referenceOutput);
    }

    if (!SparkChangesetWriter().isSupported(changesetOutput))
    {
      throw HootException(
        getName() + " only supports a .spark.x file for changeset output.  Specified output: " +
        changesetOutput);
    } 

    //inputs must be sorted by element id for changeset derivation to work
    conf().set(ConfigOptions::getApiDbReaderSortByIdKey(), true);
    //in order for the sorting to work, all original element ids must be retained...no new ones
    //assigned; we're assuming no duplicate ids in the input
    conf().set(ConfigOptions::getHootapiDbWriterRemapIdsKey(), false);
    //translate inputs to OSM
    conf().set(ConfigOptions::getTranslationScriptKey(), "translations/OSM_Ingest.js");

    LOG_INFO(
      "Ingesting Multiary data from " << newInput << " into reference output layer: " <<
      referenceOutput << " and writing changes to changeset file: " << changesetOutput << "...");

    HootApiDb referenceDb;
    referenceDb.open(referenceOutput);
    const QStringList dbUrlParts = referenceOutput.split("/");
    if (!referenceDb.mapExists(dbUrlParts[dbUrlParts.size() - 1]))
    {
      LOG_INFO("Generating a changeset from the input data only.");

      //If there's no existing reference data, then there's no point in sorting input data or
      //deriving a changeset diff.  So in that case, write all of the input data directly to the
      //ref layer and generate a Spark changeset consisting entirely of create statements.
      _sortInput = false;
      _writeChanges(_getNewInputStream(newInput), referenceOutput, changesetOutput);
    }
    else
    {
      LOG_INFO("Deriving a changeset between the input and reference data.");

      //sort incoming data by element id, if necessary, for changeset derivation (only passing nodes
      //through, so don't need to also sort by element type)
      _sortedNewInput = _getSortedNewInput(newInput);

      //create the changes and write them to the ref db layer and also to a changeset file for
      //external use by Spark
      _deriveAndWriteChanges(_getNewInputStream(_sortedNewInput), referenceOutput, changesetOutput);
    }

    LOG_INFO(
      "Multiary data ingested from " << newInput << " into reference output layer: " <<
      referenceOutput << " and changes written to changeset file: " << changesetOutput <<
      ".  Time: " << FileUtils::secondsToDhms(_timer.elapsed()));
    LOG_INFO("Changes Parsed: " << FileUtils::formatPotentiallyLargeNumber(_changesParsed));

    return 0;
  }

private:

  bool _sortInput;
  QString _sortedNewInput;
  bool _isGeoNamesInput;

  long _changesParsed;

  QElapsedTimer _timer;

  QString _getSortedNewInput(const QString newInput)
  {
    if (!_sortInput)
    {
      return newInput;
    }

    LOG_DEBUG("Retrieving input stream...");

    //write the unsorted input to a temp db layer; later it will be queried back out sorted by id

    //TODO: if performance for this ends up being a bottleneck, will later implement something here
    //that performs faster - either a file based merge sort, or using the db like this but with sql
    //copy statements instead

    boost::shared_ptr<PartialOsmMapReader> unsortedNewInputReader =
      boost::dynamic_pointer_cast<PartialOsmMapReader>(
        OsmMapReaderFactory::getInstance().createReader(newInput));
    unsortedNewInputReader->setUseDataSourceIds(true);
    unsortedNewInputReader->open(newInput);
    boost::shared_ptr<ElementInputStream> unsortedNewInputStream =
      boost::dynamic_pointer_cast<ElementInputStream>(unsortedNewInputReader);

    LOG_DEBUG("Retrieving filtered input stream...");

    boost::shared_ptr<ElementInputStream> filteredUnsortedNewInputStream =
      _getFilteredNewInputStream(unsortedNewInputStream);

    LOG_DEBUG("Retrieving output stream...");

    const QString sortedNewInput =
      HootApiDb::getBaseUrl().toString() + "/MultiaryIngest-tempNewInput-" +
      QUuid::createUuid().toString();
    boost::shared_ptr<HootApiDbWriter> unsortedNewInputWriter(new HootApiDbWriter());
    unsortedNewInputWriter->setCreateUser(true);
    unsortedNewInputWriter->setOverwriteMap(true);
    unsortedNewInputWriter->open(sortedNewInput);
    boost::shared_ptr<ElementOutputStream> unsortedNewOutputStream =
      boost::dynamic_pointer_cast<ElementOutputStream>(unsortedNewInputWriter);

    LOG_INFO(
      "Writing Multiary new input data to temp location for sorting: " << sortedNewInput << "...");
    _timer.restart();
    ElementOutputStream::writeAllElements(
      *filteredUnsortedNewInputStream, *unsortedNewOutputStream);
    LOG_INFO(
      "Multiary new input data written to temp location for sorting: " << sortedNewInput <<
      ".  Time: " << FileUtils::secondsToDhms(_timer.elapsed()));

    return sortedNewInput;
  }

  boost::shared_ptr<ElementInputStream> _getNewInputStream(const QString sortedNewInput)
  {
    boost::shared_ptr<PartialOsmMapReader> newInputReader =
      boost::dynamic_pointer_cast<PartialOsmMapReader>(
        OsmMapReaderFactory::getInstance().createReader(sortedNewInput));
    newInputReader->setUseDataSourceIds(true);
    newInputReader->open(sortedNewInput);
    return boost::dynamic_pointer_cast<ElementInputStream>(newInputReader);
  }

  boost::shared_ptr<ElementInputStream> _getFilteredNewInputStream(
    boost::shared_ptr<ElementInputStream> inputStream)
  {
    boost::shared_ptr<PoiCriterion> elementCriterion;
    //as the incoming data is read, filter it down to POIs only and translate each element;
    if (!_isGeoNamesInput) //all geonames are pois, so skip the element criterion filtering expense
    {
      elementCriterion.reset(new PoiCriterion());
    }
    QList<ElementVisitorPtr> visitors;
    visitors.append(boost::shared_ptr<TranslationVisitor>(new TranslationVisitor()));
    visitors.append(boost::shared_ptr<CalculateHashVisitor2>(new CalculateHashVisitor2()));
    boost::shared_ptr<ElementInputStream> filteredNewInputStream(
      new ElementCriterionVisitorInputStream(inputStream, elementCriterion, visitors));
    return filteredNewInputStream;
  }

  void _writeChanges(boost::shared_ptr<ElementInputStream> newInputStream,
                     const QString referenceOutput, const QString changesetOutput)
  {
    _timer.restart();

    boost::shared_ptr<ElementInputStream> filteredNewInputStream =
      _getFilteredNewInputStream(newInputStream);

    HootApiDbWriter referenceWriter;
    referenceWriter.setCreateUser(true);
    referenceWriter.setOverwriteMap(true);
    referenceWriter.open(referenceOutput);

    SparkChangesetWriter changesetFileWriter;
    changesetFileWriter.open(changesetOutput);

    while (filteredNewInputStream->hasMoreElements())
    {
      ElementPtr element = filteredNewInputStream->readNextElement();

      referenceWriter.writeElement(element);
      changesetFileWriter.writeChange(Change(Change::Create, element));
      _changesParsed++;
    }
  }

  void _deriveAndWriteChanges(boost::shared_ptr<ElementInputStream> newInputStream,
                              const QString referenceOutput, const QString changesetOutput)
  {
    _timer.restart();

    boost::shared_ptr<HootApiDbReader> referenceReader(new HootApiDbReader());
    referenceReader->setUseDataSourceIds(true);
    referenceReader->open(referenceOutput);

    ChangesetDeriverPtr changesetDeriver(
      new ChangesetDeriver(
        boost::dynamic_pointer_cast<ElementInputStream>(referenceReader), newInputStream));

    HootApiDbWriter referenceChangeWriter;
    referenceChangeWriter.setCreateUser(false);
    referenceChangeWriter.setOverwriteMap(false);
    referenceChangeWriter.open(referenceOutput);

    SparkChangesetWriter changesetFileWriter;
    changesetFileWriter.open(changesetOutput);

    while (changesetDeriver->hasMoreChanges())
    {
      const Change change = changesetDeriver->readNextChange();
      if (change.getType() != Change::Unknown)
      {
        changesetFileWriter.writeChange(change);
        referenceChangeWriter.writeChange(change);
        _changesParsed++;
      }
    }
  }
};

HOOT_FACTORY_REGISTER(Command, MultiaryIngestCmd)

}