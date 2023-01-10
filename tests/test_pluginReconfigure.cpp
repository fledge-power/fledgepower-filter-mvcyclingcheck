#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filterMvCyclingCheck.h>
#include <jsonToDatapoints.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;

static string nameReading = "data_test";

static string reconfigure = QUOTE({
    "enable": {
        "value": "false"
    }
});

static string reconfigureCheckPeriod = QUOTE({
    "check_period": {
        "value": "45"
    },
    "enable": {
        "value": "true"
    }
});


static string jsonMessagePivotMVNormal = QUOTE({
	"PIVOT": {
        "GTIM": {
            "Cause": {
                "stVal": 1
            },
            "Identifier": "M_2367_3_15_4",
            "MvTyp": {
                "mag": {
                    "i": 2
                },
                 "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "SecondSinceEpoch": 1668759955
                }
            },            
            "TmOrg": {
                "stVal": "genuine"
            },
            "TmValidity": {
                "stVal": "VALID"
            }
        }
    }
});

extern "C" {
	PLUGIN_INFORMATION *plugin_info();
	void plugin_ingest(void *handle, READINGSET *readingSet);
	PLUGIN_HANDLE plugin_init(ConfigCategory *config,
			  OUTPUT_HANDLE *outHandle,
			  OUTPUT_STREAM output);
	
	void HandlerReconfigure(void *handle, READINGSET *readings) {
		*(READINGSET **)handle = readings;
	}

    void plugin_reconfigure(PLUGIN_HANDLE *handle, const string& newConfig);
};

class PluginReconfigure : public testing::Test
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
		
		void *handle = plugin_init(config, &resultReading, HandlerReconfigure);
		filter = (FilterMvCyclingCheck *) handle;
    }

    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }   
};

TEST_F(PluginReconfigure, Reconfigure) 
{
	plugin_reconfigure((PLUGIN_HANDLE*)filter, reconfigure);
    ASSERT_EQ(filter->isEnabled(), false);

    // Create Reading
   	Datapoints *p = parseJson(jsonMessagePivotMVNormal.c_str());
	Reading *reading = new Reading(nameReading, *p);
    Readings *readings = new Readings;
    readings->push_back(reading);

    // Create ReadingSet
    ReadingSet *readingSet = new ReadingSet(readings);

	plugin_ingest(filter, (READINGSET*)readingSet);
	Readings results = resultReading->getAllReadings();
	ASSERT_EQ(results.size(), 1);

    delete reading;
}

TEST_F(PluginReconfigure, ReconfigureCheckPeriod) 
{
	plugin_reconfigure((PLUGIN_HANDLE*)filter, reconfigureCheckPeriod);
    ASSERT_EQ(filter->getThread().getCheckPeriod(), 45);
}