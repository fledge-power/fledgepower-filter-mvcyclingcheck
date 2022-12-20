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

static string jsonMessageLightMessage = QUOTE({
	"PIVOT": {
        "GTIM": {
            "Cause": {
                "stVal": 1
            },
            "Identifier": "M_2367_3_15_4",
            "MvTyp": {
                "mag": {
                    "f": 2
                }
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
 
class ThreadCyclingCheck : public testing::Test
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

    void startTest(string json) {
        // Reconfigure timeout
        filter->getThread().setCheckPeriod(0);
        
        // Convert message
        ASSERT_EQ(filter->getThread().getStore().getMap().size(), 0);
        Datapoints *dpsMessage = parseJson(json);
        Datapoint *dp = findDatapointElement(dpsMessage, ConstantsMvCyclingCheck::KeyMessagePivotJsonRoot);

        // Store messsage1
        filter->getThread().receiveMv(nameReading, dp, "test");

        ASSERT_EQ(filter->getThread().getStore().getMap().size(), 1);
        ASSERT_EQ(filter->getThread().getReadingsNoRenewal().size(), 0);

        // sleep 1.2s
        this_thread::sleep_for(chrono::milliseconds(1200));

        // Check no renewal
        ASSERT_EQ(filter->getThread().getReadingsNoRenewal().size(), 1); 
        ASSERT_EQ(filter->getThread().getStore().getMap().size(), 0);

        // Verification output Reading
        Reading *out = filter->getThread().getReadingsNoRenewal().at(0);
        ASSERT_NE(out, nullptr);

        ASSERT_STREQ(out->getAssetName().c_str(), nameReading.c_str());
        ASSERT_EQ(out->getDatapointCount(), 1);
        
        Datapoints points = out->getReadingData();
        ASSERT_EQ(points.size(), 1);

        // Get PIVOT
        Datapoints *dpsPivot = findDictElement(&points, ConstantsMvCyclingCheck::KeyMessagePivotJsonRoot);
        ASSERT_NE(dpsPivot, nullptr);

        // Get PIVOT.GTIM
        Datapoints *dpsGi = findDictElement(dpsPivot, ConstantsMvCyclingCheck::KeyMessagePivotJsonGt);
        ASSERT_NE(dpsGi, nullptr);
        
        // Get PIVOT.GTIM.MvTyp
        Datapoints *dspMvTyp = findDictElement(dpsGi, ConstantsMvCyclingCheck::KeyMessagePivotJsonCdcMv);
        ASSERT_NE(dspMvTyp, nullptr);  

        // Get PIVOT.GTIM.MvTyp.q
        Datapoints *dpsQ = findDictElement(dspMvTyp, ConstantsMvCyclingCheck::KeyMessagePivotJsonQ);
        ASSERT_NE(dpsQ, nullptr);
        
        // Get PIVOT.GTIM.MvTyp.q.DetailQuality
        Datapoints *dpsDetailQuality = findDictElement(dpsQ, ConstantsMvCyclingCheck::KeyMessagePivotJsonDetailQuality);
        ASSERT_NE(dpsDetailQuality, nullptr);

        // Verify PIVOT.GTIM.MvTyp.q.DetailQuality.OldData at value 1
        DatapointValue *dvOldData = findValueElement(dpsDetailQuality, ConstantsMvCyclingCheck::KeyMessagePivotJsonOldData);
        ASSERT_NE(dvOldData, nullptr);
        ASSERT_EQ(dvOldData->toInt(), 1);

        // Verify PIVOT.GTIM.MvTyp.q.Validity at value "questionable"
        DatapointValue *dvValidity = findValueElement(dpsQ, ConstantsMvCyclingCheck::KeyMessagePivotJsonValidity);
        ASSERT_NE(dvValidity, nullptr);
        ASSERT_EQ(dvValidity->toStringValue(), ConstantsMvCyclingCheck::ValueQuestionable.c_str());

        // Verify PIVOT.GTIM.MvTyp.q.Source at value "substituted"
        DatapointValue *dvSource = findValueElement(dpsQ, ConstantsMvCyclingCheck::KeyMessagePivotJsonSource);
        ASSERT_NE(dvSource, nullptr);
        ASSERT_EQ(dvSource->toStringValue(), ConstantsMvCyclingCheck::ValueSubstituted.c_str());

        // Get PIVOT.GTIM.MvTyp.t
        Datapoints *dspT = findDictElement(dspMvTyp, ConstantsMvCyclingCheck::KeyMessagePivotJsonTs);
        ASSERT_NE(dspT, nullptr);  

        // Verify PIVOT.GTIM.MvTyp.t.SecondSinceEpoch is superior at the value of first message
        DatapointValue *dvSinceSecondEpoch = findValueElement(dspT, ConstantsMvCyclingCheck::KeyMessagePivotJsonSecondSinceEpoch);
        ASSERT_NE(dvSinceSecondEpoch, nullptr);
        ASSERT_TRUE(dvSinceSecondEpoch->toInt() > 1668759955);

        // Verify PIVOT.GTIM.MvTyp.t.FractionOfSecond is different at the value of first message
        DatapointValue *dvFractionOfSecond = findValueElement(dspT, ConstantsMvCyclingCheck::KeyMessagePivotJsonFractionOfSecond);
        ASSERT_NE(dvFractionOfSecond, nullptr);
        ASSERT_TRUE(dvFractionOfSecond->toInt() != 95685);

        // Get PIVOT.GTIM.TmOrg
        Datapoints *dspTmOrg = findDictElement(dpsGi, ConstantsMvCyclingCheck::KeyMessagePivotJsonTmOrg);
        ASSERT_NE(dspTmOrg, nullptr);

        // Verify PIVOT.GTIM.TmOrg.stVal at value "substituted"
        DatapointValue *dvTmOrgStVal = findValueElement(dspTmOrg, ConstantsMvCyclingCheck::KeyMessagePivotJsonStVal);
        ASSERT_NE(dvTmOrgStVal, nullptr);
        ASSERT_EQ(dvTmOrgStVal->toStringValue(), ConstantsMvCyclingCheck::ValueSubstituted.c_str());

        // Get PIVOT.GTIM.TmValidity
        Datapoints *dspValidity = findDictElement(dpsGi, ConstantsMvCyclingCheck::KeyMessagePivotJsonTmValidity);
        ASSERT_NE(dspTmOrg, nullptr);

        // Verify PIVOT.GTIM.TmValidity.stVal at value "valid"
        DatapointValue *dvTmValidityStVal = findValueElement(dspValidity, ConstantsMvCyclingCheck::KeyMessagePivotJsonStVal);
        ASSERT_NE(dvTmValidityStVal, nullptr);
        ASSERT_EQ(dvTmValidityStVal->toStringValue(), ConstantsMvCyclingCheck::ValueValid.c_str());

        // Get PIVOT.GTIM.Cause
        Datapoints *dspCause = findDictElement(dpsGi, ConstantsMvCyclingCheck::KeyMessagePivotJsonCot);
        ASSERT_NE(dspTmOrg, nullptr);

        // Verify PIVOT.GTIM.Cause.stVal at value 3
        DatapointValue *dvCauseStVal = findValueElement(dspCause, ConstantsMvCyclingCheck::KeyMessagePivotJsonStVal);
        ASSERT_NE(dvCauseStVal, nullptr);
        ASSERT_EQ(dvCauseStVal->toInt(), 3);

        // Clear vector
        filter->getThread().clearReadingsNoRenewal();
        ASSERT_EQ(filter->getThread().getReadingsNoRenewal().size(), 0); 
        ASSERT_EQ(filter->getThread().getStore().getMap().size(), 0);
    }
};

TEST_F(ThreadCyclingCheck, testThread)
{
    startTest(jsonMessage);
}

TEST_F(ThreadCyclingCheck, testThreadLightMessage)
{
    startTest(jsonMessageLightMessage);
}

TEST_F(ThreadCyclingCheck, StopAndStartThread)
{
    ASSERT_TRUE(filter->getThread().getStarted());

    filter->getThread().stop();

    ASSERT_FALSE(filter->getThread().getStarted());

    filter->getThread().start();

    ASSERT_TRUE(filter->getThread().getStarted());
}