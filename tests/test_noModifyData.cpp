#include <gmock/gmock.h>
#include <gtest/gtest.h>
 
#include <filterMvCyclingCheck.h>
#include <jsonToDatapoints.h>
#include <constantsMvCyclingCheck.h>

using namespace std;
using namespace DatapointUtility;
using namespace JsonToDatapoints;
 
static string nameReading = "data_test";
 
static string jsonMessagePivotTS = QUOTE({
	"PIVOTTS": {
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
 
static string jsonMessageGTIS = QUOTE({
    "PIVOT": {
        "GTIS": {
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
 
static string jsonMessageSpsTyp = QUOTE({
    "PIVOT": {
        "GTIM": {
            "Cause": {
                "stVal": 1
            },
            "Identifier": "M_2367_3_15_4",
            "SpsTyp": {
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
 
static string jsonMessageWithoutID = QUOTE({
    "PIVOT": {
        "GTIM": {
            "Cause": {
                "stVal": 1
            },
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

static string jsonMessageWithoutCause = QUOTE({
    "PIVOT": {
        "GTIM": {
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

static string jsonMessageWithoutStValCause = QUOTE({
    "PIVOT": {
        "GTIM": {
            "Cause": {},
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

static string jsonMessageCauseAt2 = QUOTE({
	"PIVOT": {
        "GTIM": {
            "Cause": {
                "stVal": 2
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
   
    void HandlerNoModifyData(void *handle, READINGSET *readings) {
        *(READINGSET **)handle = readings;
    }
};
 
class NoModifyData : public testing::Test
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
       
        void *handle = plugin_init(config, &resultReading, HandlerNoModifyData);
        filter = (FilterMvCyclingCheck *) handle;
    }
 
    // TearDown is ran for every tests, so each variable are destroyed again
    void TearDown() override
    {
        delete filter;
    }
 
    void startTests(string json, std::string namePivotData, std::string nameGi, std::string nameTyp) {
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
 
        verifyDatapoint(&points, namePivotData, nameGi, nameTyp, json);
 
        delete reading;
    }
 
    void verifyDatapoint(Datapoints *dps, std::string namePivotData, std::string nameGi, std::string nameTyp, std::string json) {
        Datapoints *dpPivot = findDictElement(dps, namePivotData);
        ASSERT_NE(dpPivot, nullptr);
       
        Datapoints *dpGi = findDictElement(dpPivot, nameGi);
        ASSERT_NE(dpGi, nullptr);
        
        Datapoints *MvTyp = findDictElement(dpGi, nameTyp);
        ASSERT_NE(MvTyp, nullptr);  
 
        Datapoints *dpT = findDictElement(MvTyp, ConstantsMvCyclingCheck::KeyMessagePivotJsonTs);
        ASSERT_NE(dpT, nullptr);
 
        DatapointValue *sinceSecondEpoch = findValueElement(dpT, ConstantsMvCyclingCheck::KeyMessagePivotJsonSecondSinceEpoch);
        ASSERT_NE(sinceSecondEpoch, nullptr);
        ASSERT_EQ(sinceSecondEpoch->toInt(), 1668759955);
    
        DatapointValue *fractionOfSecond = findValueElement(dpT, ConstantsMvCyclingCheck::KeyMessagePivotJsonFractionOfSecond);
        ASSERT_NE(fractionOfSecond, nullptr);
        ASSERT_EQ(fractionOfSecond->toInt(), 95685);

        if (json != jsonMessageWithoutCause && json != jsonMessageWithoutStValCause) {
            Datapoints *dpsCause = findDictElement(dpGi, ConstantsMvCyclingCheck::KeyMessagePivotJsonCot);
            ASSERT_NE(dpsCause, nullptr);

            DatapointValue *dpvstValCause = findValueElement(dpsCause, ConstantsMvCyclingCheck::KeyMessagePivotJsonStVal);
            ASSERT_NE(dpvstValCause, nullptr);

            if (json == jsonMessageCauseAt2) {
                ASSERT_EQ(dpvstValCause->toInt(), 2);
            }
            else {
                ASSERT_EQ(dpvstValCause->toInt(), 1);
            }
        }
    }
};

TEST_F(NoModifyData, MessagePIVOTTS)
{
    startTests(jsonMessagePivotTS, "PIVOTTS", "GTIM", "MvTyp");
}
 
TEST_F(NoModifyData, MessageGTIS)
{
    startTests(jsonMessageGTIS, "PIVOT", "GTIS", "MvTyp");
}
 
TEST_F(NoModifyData, MessageSpsTyp)
{
    startTests(jsonMessageSpsTyp, "PIVOT", "GTIM", "SpsTyp");
}
 
TEST_F(NoModifyData, MessageWithoutID)
{
    startTests(jsonMessageWithoutID, "PIVOT", "GTIM", "MvTyp");
}
 
TEST_F(NoModifyData, MessageWithoutCause)
{
    startTests(jsonMessageWithoutCause, "PIVOT", "GTIM", "MvTyp");
}

TEST_F(NoModifyData, MessageWithoutStValCause)
{
    startTests(jsonMessageWithoutStValCause, "PIVOT", "GTIM", "MvTyp");
}

TEST_F(NoModifyData, MessageCauseAt2)
{
    startTests(jsonMessageCauseAt2, "PIVOT", "GTIM", "MvTyp");
}
