#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterMvCyclingCheck.h>

using namespace std;

extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	PLUGIN_HANDLE plugin_init(ConfigCategory* config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);

    void plugin_shutdown(PLUGIN_HANDLE *handle);
};

class PluginShutdown : public testing::Test
{
protected:
    FilterMvCyclingCheck * filter = nullptr;  // Object on which we call for tests
    ReadingSet * resultReading;

    // Setup is ran for every tests, so each variable are reinitialised
    void SetUp() override
    {
        PLUGIN_INFORMATION *info = plugin_info();
		ConfigCategory *config = new ConfigCategory("mvcyclingcheck", info->config);
		
		ASSERT_NE(config, (ConfigCategory *)NULL);		
		config->setItemsValueFromDefault();
		config->setValue("enable", "true");
		
		void *handle = plugin_init(config, &resultReading, nullptr);
		filter = (FilterMvCyclingCheck *) handle;
    }
};

TEST_F(PluginShutdown, Shutdown) 
{
	ASSERT_NO_THROW(plugin_shutdown((PLUGIN_HANDLE*)filter));
}