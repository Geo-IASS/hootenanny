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
#ifndef THREADED_TEST_SUITE_H
#define THREADED_TEST_SUITE_H

// CPP Unit
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>

//  Boost
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>

namespace hoot
{

class ThreadedTestSuite : public CppUnit::TestSuite
{
public:
  ThreadedTestSuite(std::string name, int threads = 0)
    : CppUnit::TestSuite(name),
      _threads(threads ? threads : boost::thread::hardware_concurrency())
  {
  }

  void doRunChildTests(CppUnit::TestResult* controller)
  {
    boost::asio::io_service io;
    boost::thread_group threadpool;
    boost::asio::io_service::work work(io);

    for (int i = 0; i < _threads; ++i)
      threadpool.create_thread(boost::bind(&boost::asio::io_service::run, &io));

    for (int i = 0; i < getChildTestCount(); i++)
      io.post(boost::bind(threadTest, getChildTestAt(i), controller));

    io.stop();

    threadpool.join_all();
  }

private:
  static void threadTest(CppUnit::Test* test, CppUnit::TestResult* controller)
  {
    test->run(controller);
  }

  const int _threads;
};

//  Might not be necessary
#define CPPUNIT_TEST_SUITE_END_THREADED()                               \
    }                                                                   \
    static CppUnit::TestSuite *suite()                                  \
    {                                                                   \
      const CppUnit::TestNamer &namer = getTestNamer__();               \
      std::auto_ptr<CppUnit::TestSuite> suite(                          \
         new hoot::ThreadedTestSuite( namer.getFixtureName()));         \
      CppUnit::ConcretTestFixtureFactory<TestFixtureType> factory;      \
      CppUnit::TestSuiteBuilderContextBase context( *suite.get(),       \
                               namer,                                   \
                               factory );                               \
      TestFixtureType::addTestsToSuite( context );                      \
      return suite.release();                                           \
    }                                                                   \
  private: /* dummy typedef so that the macro can still end with ';'*/  \
    typedef int CppUnitDummyTypedefForSemiColonEnding__
}

#endif // THREADED_TEST_SUITE_H
