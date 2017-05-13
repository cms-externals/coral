// Include files
#include <boost/assign/list_of.hpp>
#include "CoralBase/../tests/Common/CoralCppUnitTest.h"
#include "CoralKernel/Context.h"
#include "CoralKernel/ILoadableComponent.h"

// Local include files
#include "../../src/PluginManager.h"

// Forward declaration (for easier indentation)
namespace coral
{
  class ExternalPluginManagerTest;
}

// Test for external plugin managers
// Author: Zsolt Molnar
class coral::ExternalPluginManagerTest : public coral::CoralCppUnitTest
{

  CPPUNIT_TEST_SUITE( ExternalPluginManagerTest );
  CPPUNIT_TEST( test_loadedComponents_bug43956 );
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() {}

  void tearDown() {}

  // ------------------------------------------------------

  static const std::set<std::string>& ComponentNameFixture()
  {
    // First component is from PluginManager, second from TestPluginManager
    static const std::set<std::string> f = boost::assign::list_of
      ("CORAL/RelationalPlugins/sqlite") ("TestComponent");
    return f;
  }

  /// Test plugin manager, with knownPlugin overridden to return fixtures.
  struct TestPluginManager : public IPluginManager
  {

    std::set<std::string> knownPlugins() const
    {
      // First component is from PluginManager, second from TestPluginManager
      static const std::set<std::string> f = boost::assign::list_of
        (*(++(ComponentNameFixture().begin())));
      return f;
    }

    ILoadableComponent* newComponent( const std::string& /*componentName*/ )
    {
      return new TestLoadableComponent;
    }

  private:

    struct TestLoadableComponent : public ILoadableComponent
    {
      TestLoadableComponent() : coral::ILoadableComponent( "Dummy" )
      {
      }
    };

  };

  // Test loaded components after adding new plugin manager (bug #43956)
  void test_loadedComponents_bug43956()
  {
    Context& ctx = Context::instance();
    // Record the component names known by the default plugin manager
    std::set<std::string> knownComp(ctx.knownComponents());
    CPPUNIT_ASSERT(!knownComp.empty());
    // No plugin is loaded yet, so the list of known plugins must be empty
    CPPUNIT_ASSERT(ctx.loadedComponents().empty());
    // Load a plugin, by using the default plugin manager
    ctx.loadComponent(*ComponentNameFixture().begin());
    std::set<std::string> loadedComp(ctx.loadedComponents());
    // The component name must be known here, but only this
    CPPUNIT_ASSERT_EQUAL(size_t(1), loadedComp.size());
    CPPUNIT_ASSERT_EQUAL(*loadedComp.begin(), *ComponentNameFixture().begin());
    // Now load a component, by using a different plugin manager
    ctx.loadComponent( *(++(ComponentNameFixture().begin())),
                       new TestPluginManager );
    loadedComp = ctx.loadedComponents();
    // Both the components name must be visible
    CPPUNIT_ASSERT(loadedComp == ComponentNameFixture());
    // Check if the known components did not altered
    CPPUNIT_ASSERT(knownComp == ctx.knownComponents());
  }

  // ------------------------------------------------------


  // ------------------------------------------------------

};

CPPUNIT_TEST_SUITE_REGISTRATION( coral::ExternalPluginManagerTest );

CORALCPPUNITTEST_MAIN( ExternalPluginManagerTest )
