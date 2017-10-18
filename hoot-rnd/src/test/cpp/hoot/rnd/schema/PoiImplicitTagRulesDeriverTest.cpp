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
#include <hoot/core/TestUtils.h>
#include <hoot/rnd/schema/PoiImplicitTagRulesDeriver.h>
#include <hoot/rnd/io/ImplicitTagRulesSqliteReader.h>

// Qt
#include <QDir>

namespace hoot
{

class PoiImplicitTagRulesDeriverTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PoiImplicitTagRulesDeriverTest);
  //CPPUNIT_TEST(hashTest);
  CPPUNIT_TEST(runBasicTest);
//  CPPUNIT_TEST(runTypeKeysTest);
//  CPPUNIT_TEST(runMinOccuranceThresholdTest);
//  CPPUNIT_TEST(runMultipleInputsTest);
//  CPPUNIT_TEST(runNameCaseTest);
  //TODO
  //CPPUNIT_TEST(runInputTranslationScriptSizeMismatchTest);
  //CPPUNIT_TEST(runEqualsInNameTest);
  CPPUNIT_TEST_SUITE_END();

public:

  static QString inDir() { return "test-files/io/PoiImplicitTagRulesDeriverTest"; }
  static QString outDir() { return "test-output/io/PoiImplicitTagRulesDeriverTest"; }

//  void hashTest()
//  {
//    PoiImplicitTagRulesDeriver rulesDeriver;
//    //FixedLengthString fls = rulesDeriver._qStrToFixedLengthStr2("test");
//    LOG_VART(rulesDeriver._qStrToFixedLengthStr2("test").data);
//    LOG_VART(rulesDeriver._qStrToFixedLengthStr2("test").data);
//    LOG_VART(rulesDeriver._qStrToFixedLengthStr2("test2").data);
//    //map->getNode(-5)->getTags()["alt_name"] = QString::fromUtf8("Şiḩḩī");
//    LOG_VART(rulesDeriver._qStrToFixedLengthStr2("Şiḩḩī").data);

//    //LOG_VART(QString::fromUtf8(rulesDeriver._fixedLengthStrToQStr(fls).toStdString().data()));
//    //LOG_VART(rulesDeriver._fixedLengthStrToQStr2(fls));

//    /*FixedLengthString fls2 = rulesDeriver._qStrToFixedLengthStr2("test");
//    LOG_VART(fls2.data);
//    FixedLengthString fls3 = rulesDeriver._qStrToFixedLengthStr2("test2");
//    LOG_VART(fls3.data);*/
//    //CPPUNIT_ASSERT_EQUAL(fls.data, fls2.data);
//    //QString str = rulesDeriver._fixedLengthStrToQStr2(fls);
//    //LOG_VART(str);

////    FixedLengthString fls4 = rulesDeriver._qStrToFixedLengthStr("test");
////    LOG_VART(fls4.data);
////    FixedLengthString fls5 = rulesDeriver._qStrToFixedLengthStr("test");
////    LOG_VART(fls5.data);
////    FixedLengthString fls6 = rulesDeriver._qStrToFixedLengthStr("test2");
////    LOG_VART(fls6.data);

////    uint test = qHash(QString("test"));
////    LOG_VART(test);
////    uint test2 = qHash(QString("test2"));
////    LOG_VART(test2);
////    CPPUNIT_TEST(qHash(QString("test2"));
//  }

  void runBasicTest()
  {
    QDir().mkpath(outDir());

    QStringList inputs;
    inputs.append(inDir() + "/yemen-crop-2.osm.pbf");
    const QString jsonOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runBasicTest-out.json";
    const QString dbOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runBasicTest-out.sqlite";

    QStringList translationScripts;
    translationScripts.append("translations/OSM_Ingest.js");

    QStringList outputs;
    outputs.append(jsonOutputFile);
    outputs.append(dbOutputFile);

    PoiImplicitTagRulesDeriver().deriveRules(inputs, translationScripts, outputs);

//    HOOT_FILE_EQUALS(inDir() + "/PoiImplicitTagRulesDeriverTest-runBasicTest.json", jsonOutputFile);

//    ImplicitTagRulesSqliteReader dbReader;
//    dbReader.open(dbOutputFile);
//    CPPUNIT_ASSERT_EQUAL(441L, dbReader.getRuleCount());
//    dbReader.close();
  }

  void runTypeKeysTest()
  {
    QDir().mkpath(outDir());

    QStringList inputs;
    inputs.append(inDir() + "/yemen-crop-2.osm.pbf");

    QStringList translationScripts;
    translationScripts.append("translations/OSM_Ingest.js");

    const QString jsonOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runTypeKeysTest-out.json";
    const QString dbOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runTypeKeysTest-out.sqlite";
    QStringList outputs;
    outputs.append(jsonOutputFile);
    outputs.append(dbOutputFile);

    QStringList typeKeys;
    typeKeys.append("amenity");
    typeKeys.append("tourism");
    typeKeys.append("building");

    PoiImplicitTagRulesDeriver().deriveRules(inputs, translationScripts, outputs, typeKeys);

    HOOT_FILE_EQUALS(
      inDir() + "/PoiImplicitTagRulesDeriverTest-runTypeKeysTest.json", jsonOutputFile);

    ImplicitTagRulesSqliteReader dbReader;
    dbReader.open(dbOutputFile);
    CPPUNIT_ASSERT_EQUAL(247L, dbReader.getRuleCount());
    dbReader.close();
  }

  void runMinOccuranceThresholdTest()
  {
    QDir().mkpath(outDir());

    QStringList inputs;
    inputs.append(inDir() + "/yemen-crop-2.osm.pbf");

    QStringList translationScripts;
    translationScripts.append("translations/OSM_Ingest.js");

    const QString jsonOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runMinOccuranceThresholdTest-out.json";
    const QString dbOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runMinOccuranceThresholdTest-out.sqlite";
    QStringList outputs;
    outputs.append(jsonOutputFile);
    outputs.append(dbOutputFile);

    PoiImplicitTagRulesDeriver().deriveRules(inputs, translationScripts, outputs, QStringList(), 4);

    HOOT_FILE_EQUALS(
      inDir() + "/PoiImplicitTagRulesDeriverTest-runMinOccuranceThresholdTest.json", jsonOutputFile);

    ImplicitTagRulesSqliteReader dbReader;
    dbReader.open(dbOutputFile);
    CPPUNIT_ASSERT_EQUAL(108L, dbReader.getRuleCount());
    dbReader.close();
  }

  void runMultipleInputsTest()
  {
    QDir().mkpath(outDir());

    QStringList inputs;
    inputs.append(inDir() + "/yemen-crop-2.osm.pbf");
    inputs.append(inDir() + "/philippines-1.osm.pbf");

    QStringList translationScripts;
    translationScripts.append("translations/OSM_Ingest.js");
    translationScripts.append("translations/OSM_Ingest.js");

    const QString jsonOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runMultipleInputsTest-out.json";
    const QString dbOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runMultipleInputsTest-out.sqlite";
    QStringList outputs;
    outputs.append(jsonOutputFile);
    outputs.append(dbOutputFile);

    PoiImplicitTagRulesDeriver().deriveRules(inputs, translationScripts, outputs);

    HOOT_FILE_EQUALS(
      inDir() + "/PoiImplicitTagRulesDeriverTest-runMultipleInputsTest.json", jsonOutputFile);

    ImplicitTagRulesSqliteReader dbReader;
    dbReader.open(dbOutputFile);
    CPPUNIT_ASSERT_EQUAL(602L, dbReader.getRuleCount());
    dbReader.close();
  }

  void runNameCaseTest()
  {
    //Case is actually already handled correctly in runBasicTest, but this smaller input dataset
    //will make debugging case problems easier, if needed.

    QDir().mkpath(outDir());

    QStringList inputs;
    inputs.append(inDir() + "/PoiImplicitTagRulesDeriverTest-runNameCaseTest.osm");

    QStringList translationScripts;
    translationScripts.append("translations/OSM_Ingest.js");

    const QString jsonOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runNameCaseTest-out.json";
    const QString dbOutputFile =
      outDir() + "/PoiImplicitTagRulesDeriverTest-runNameCaseTest-out.sqlite";
    QStringList outputs;
    outputs.append(jsonOutputFile);
    outputs.append(dbOutputFile);

    PoiImplicitTagRulesDeriver().deriveRules(inputs, translationScripts, outputs);

    HOOT_FILE_EQUALS(
      inDir() + "/PoiImplicitTagRulesDeriverTest-runNameCaseTest.json", jsonOutputFile);

    ImplicitTagRulesSqliteReader dbReader;
    dbReader.open(dbOutputFile);
    CPPUNIT_ASSERT_EQUAL(9L, dbReader.getRuleCount());
    dbReader.close();
  }
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(PoiImplicitTagRulesDeriverTest, "quick");

}
