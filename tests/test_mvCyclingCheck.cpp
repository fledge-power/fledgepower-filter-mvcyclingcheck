#include <gmock/gmock.h>
#include <gtest/gtest.h>
 
#include <filterMvCyclingCheck.h>
#include <jsonToDatapoints.h>
#include <constantsMvCyclingCheck.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;
 
static string nameReading = "data_test";

static string configure = QUOTE({
    "check_period": {
        "value": "0"
    },
    "enable": {
        "value": "true"
    }
});

static string jsonMessage = QUOTE({
	"PIVOT": {
        "GTIM": {
            "Cause": {
                "stVal": 1
            },
            "Identifier": "M_2367_3_15_4",
            "MvTyp": {
                "mag": {
                    "f": 2
                },
                 "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "SecondSinceEpoch": 1668759955,
                    "FractionOfSecond": 95685
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

static string jsonMessageWithCauseFloat = QUOTE({
	"PIVOT": {
        "GTIM": {
            "Cause": {
                "stVal": 1.0
            },
            "Identifier": "M_2367_3_15_4",
            "MvTyp": {
                "mag": {
                    "f": 2
                },
                 "q": {
                    "Source": "process",
                    "Validity": "good"
                },
                "t": {
                    "SecondSinceEpoch": 1668759955,
                    "FractionOfSecond": 95685
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
   
    void HandlerMvCyclingCheck(void *handle, READINGSET *readings) {
        *(READINGSET **)handle = readings;
    }
    void plugin_reconfigure(PLUGIN_HANDLE *handle, const string& newConfig);

};
 
class MvCyclingCheck : public testing::Test
{
protected:
    FilterMvCyclingCheck *filter = nullptr;  // Object on which we call for tests
    ReadingSet *resultReading;
 
    // Setup is ran for every tests, so each variable are reinitialised
    void SetUp() override
    {
        PLUGIN_INFORMATION *info = plugin_info();
        ConfigCategory *config = new ConfigCategory("mvcyclingcheck", info->config);
       
        ASSERT_NE(config, (ConfigCategory *)NULL);      
        config->setItemsValueFromDefault();
        config->setValue("enable", "true");
       
        void *handle = plugin_init(config, &resultReading, HandlerMvCyclingCheck);
        filter = (FilterMvCyclingCheck *) handle;

        plugin_reconfigure((PLUGIN_HANDLE*)filter, configure);
    }
 
    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }
 
    void startTests(string json) {
        ASSERT_NE(filter, (void *)NULL);
 
        // Create Reading
        Datapoints *p = parseJson(json);
 
        Reading *reading = new Reading(nameReading, *p);
        Readings *readings = new Readings;
        readings->push_back(reading);
 
        // Create ReadingSet
        ReadingSet *readingSet = new ReadingSet(readings);
       
        plugin_ingest(filter, (READINGSET*)readingSet);
        Readings results = resultReading->getAllReadings();
    	ASSERT_EQ(results.size(), 1);
 
        Reading *out = results[0];
        ASSERT_STREQ(out->getAssetName().c_str(), nameReading.c_str());
        ASSERT_EQ(out->getDatapointCount(), 1);
 
        Datapoints points = out->getReadingData();
        ASSERT_EQ(points.size(), 1);

        ASSERT_EQ(filter->getThread().getStore().getMap().size(), 1);

        // sleep 1.2s
        this_thread::sleep_for(chrono::milliseconds(1200));

        // Check no renewal
        ASSERT_EQ(filter->getThread().getReadingsNoRenewal().size(), 1); 
        ASSERT_EQ(filter->getThread().getStore().getMap().size(), 0);
 
        delete reading;
    }

};

TEST_F(MvCyclingCheck, Message)
{
    startTests(jsonMessage);
}
 
TEST_F(MvCyclingCheck, MessageWithCauseFloat)
{
    startTests(jsonMessageWithCauseFloat);
}
