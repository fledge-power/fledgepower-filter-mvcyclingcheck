#include <gmock/gmock.h>
#include <gtest/gtest.h>
 
#include <filterMvCyclingCheck.h>
#include <jsonToDatapoints.h>
#include <constantsMvCyclingCheck.h>
#include <utilityPivot.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;

static string nameReading = "data_test";

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

static string jsonMessage2 = QUOTE({
	"PIVOT": {
        "GTIM": {
            "Cause": {
                "stVal": 1
            },
            "Identifier": "M_2367_3_15_4",
            "MvTyp": {
                "mag": {
                    "f": 3
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
    PLUGIN_HANDLE plugin_init(ConfigCategory *config,
              OUTPUT_HANDLE *outHandle,
              OUTPUT_STREAM output);
};
 
class StoreData : public testing::Test
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
       
        void *handle = plugin_init(config, &resultReading, nullptr);
        filter = (FilterMvCyclingCheck *) handle;
    }
 
    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }
};

TEST_F(StoreData, storeData)
{
    // Convert message 1
    ASSERT_EQ(filter->getThread().getStore().getMap().size(), 0);
    Datapoints *dpsMessage1 = parseJson(jsonMessage);
    Datapoint *dp1 = findDatapointElement(dpsMessage1, ConstantsMvCyclingCheck::KeyMessagePivotJsonRoot);
    
    // Store messsage1
    filter->getThread().receiveMv(nameReading, dp1, "test");
    
    // Verify message1
    ASSERT_EQ(filter->getThread().getStore().getMap().size(), 1);    
    storeData store = filter->getThread().getStore().getMap()["test"];
    ASSERT_EQ(store.assetName, nameReading);
    ASSERT_STREQ(store.dpStored->toJSONProperty().c_str(), dp1->toJSONProperty().c_str());
    ASSERT_NE(store.timestamp, 0);

    long t1 = store.timestamp;

    // Convert message 2
    Datapoints *dpsMessage2 = parseJson(jsonMessage2);
    Datapoint *dp2 = findDatapointElement(dpsMessage2, ConstantsMvCyclingCheck::KeyMessagePivotJsonRoot);

    // sleep
    this_thread::sleep_for(chrono::milliseconds(1000));
    
    // Store messsage2
    filter->getThread().receiveMv(nameReading, dp2, "test");

    storeData store1 = filter->getThread().getStore().getMap()["test"];
    ASSERT_EQ(store1.assetName, nameReading);
    ASSERT_STREQ(store1.dpStored->toJSONProperty().c_str(), dp2->toJSONProperty().c_str());
    ASSERT_TRUE(store1.dpStored->toJSONProperty() != dp1->toJSONProperty());

    ASSERT_NE(store1.timestamp, t1);

    // Erase unkown message
    filter->getThread().getStore().eraseStoreMap("test1");
    ASSERT_EQ(filter->getThread().getStore().getMap().size(), 1);

    // Erase message
    filter->getThread().getStore().eraseStoreMap("test");
    ASSERT_EQ(filter->getThread().getStore().getMap().size(), 0);
}