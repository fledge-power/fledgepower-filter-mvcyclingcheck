/*
 * Fledge filter Measured values cycling check.
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 *
 */
 
#include <threadCheckMv.h>
#include <string>
#include <thread>
#include <chrono>
#include <map>
#include <constantsMvCyclingCheck.h>
#include <datapointUtility.h>
#include <utilityPivot.h>
 
using namespace DatapointUtility;
using namespace std;
 
/**
 * Constructor
*/
ThreadCheckMv::ThreadCheckMv() : 
        m_started(false), 
        m_checkPeriod(30),
        m_timeoutLoopThread(1000)
{
}
 
/**
 * Destructor
*/
ThreadCheckMv::~ThreadCheckMv() {
    stop();
};
 
/**
 * Start the thread  for checking the old timestamp measured
*/
void ThreadCheckMv::start() {
    if (m_started == false) {
        m_started = true;
        m_checkMv = new thread (&ThreadCheckMv::runCheckMv, this);
    }
};
 
/**
 * Stop the thread
*/
void ThreadCheckMv::stop() {
    m_started = false;
    if(m_checkMv != nullptr) {
        m_checkMv->join();
        delete m_checkMv;
        m_checkMv = nullptr;
    }
};
 
/**
 * The method sets the configured check_period.
*/
void ThreadCheckMv::setCheckPeriod(int checkPeriod) {
    m_checkPeriod = checkPeriod;
}
 
/**
 * The  method stores received data for processing in the thread.
 * 
 * @param assetName : name of asset
 * @param dict : datapoint saved
 * @param isID : identifier on value measured
 * @param isTimetamp : timestamp on value measured
*/
void ThreadCheckMv::receiveMv(const string& assetName, Datapoint *dp, const std::string& id) {
    lock_guard<mutex> guard(m_mutex);

    m_store.setOriginalDp(assetName, dp, id);
}

/**
 * The  method delete data for processing in the thread.
 * 
 * @param id : id deleted is map
*/
void ThreadCheckMv::deleteMv(const string& id) {
    lock_guard<mutex> guard(m_mutex);

    m_store.eraseStoreMap(id);
}
 
/**
 * This method executes the updated measured value and checks its timestamp whether it is old or not,
 * in order to send the non-renewal measured value.
 * Runs on thread
*/
void ThreadCheckMv::runCheckMv(){
 
    while(m_started){

        Logger::getLogger()->debug("%s runCheckMv - new loop (size list %lu)", ConstantsMvCyclingCheck::NamePlugin.c_str(), m_store.getMap().size());
         
        long currentTimestamp = UtilityPivot::getCurrentTimestampMs();        
        std::map<std::string, storeData> mapToCheckMv = m_store.getMap();

        for(auto& item: mapToCheckMv) {

            // If a measured value is not received during a timeout (current time - check period)
            // the measured value is reported as not renewed 
            if(item.second.timestamp < (currentTimestamp - m_checkPeriod * 1000)){ 
               
                string new_ID = item.first;
                storeData store = item.second;
                
                string beforeLog = ConstantsMvCyclingCheck::NamePlugin + " - " + store.assetName.c_str() + " - ingest : ";

                Datapoint *dataPointOriginal = item.second.dpStored; 
                DatapointValue dpOriginal(store.dpStored->getData());

                // Get PIVOT.GTIM
                Datapoints *dpsRecGtIM = findDictElement(dpOriginal.getDpVec(), ConstantsMvCyclingCheck::KeyMessagePivotJsonGt);
                Logger::getLogger()->debug("%s Checking stored %s", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonGt.c_str());   

                // Set PIVOT.GTIM.Cause.StVal at value 3
                Datapoints *dpsCause        = findDictElement(dpsRecGtIM, ConstantsMvCyclingCheck::KeyMessagePivotJsonCot);                 
                DatapointValue *valueStVal  = findValueElement(dpsCause, ConstantsMvCyclingCheck::KeyMessagePivotJsonStVal);

                Logger::getLogger()->debug("%s set attribute %s at value 3", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonCot.c_str());
                valueStVal->setValue((long)3);

                // Get PIVOT.GTIM.MvTyp
                Datapoints *dpsMvTyp = findDictElement(dpsRecGtIM, ConstantsMvCyclingCheck::KeyMessagePivotJsonCdcMv);

                // Get PIVOT.GTIM.MvTyp.q
                Datapoints *dpsQ = findDictElement(dpsMvTyp, ConstantsMvCyclingCheck::KeyMessagePivotJsonQ);
                if(dpsQ == nullptr){    
                    Datapoint *createDpQ = createDictElement(dpsMvTyp, ConstantsMvCyclingCheck::KeyMessagePivotJsonQ);
                    dpsQ = createDpQ->getData().getDpVec();
                }
            
                setQuality(dpsQ, beforeLog);
                setTimestampQuality(dpsRecGtIM, beforeLog);
                setTimestampMVNoRenewal(dpsMvTyp, beforeLog, currentTimestamp);                
               
                Logger::getLogger()->debug("%s data non-renewal %s, timestamp : %lu, current timestamp %lu", beforeLog.c_str(), store.dpStored->toJSONProperty().c_str(), item.second.timestamp, currentTimestamp);
 
                Datapoint *dp = new Datapoint(item.second.dpStored->getName(), dpOriginal);
                Reading *reading = new Reading(store.assetName, dp);
                m_vecReadingNoRenewal.push_back(reading);
 
                deleteMv(new_ID);
            }
        }
 
        this_thread::sleep_for(chrono::milliseconds(m_timeoutLoopThread));
    }
}

/**
 * Set quality
 * 
 * @param dpsQ : pointer of dictionary on PIVOT.GTIM.MvTyp.q
 * @param beforeLog : prefix of logs
*/
void ThreadCheckMv::setQuality(Datapoints *dpsQ, string beforeLog) {
    // Get PIVOT.GTIM.MvTyp.q.DetailQuality
    Datapoints *dpsDetailQuality = findDictElement(dpsQ, ConstantsMvCyclingCheck::KeyMessagePivotJsonDetailQuality);
    if(dpsDetailQuality == nullptr){
        Datapoint *dpDQ = createDictElement(dpsQ, ConstantsMvCyclingCheck::KeyMessagePivotJsonDetailQuality);
        dpsDetailQuality = dpDQ->getData().getDpVec();
    }
    
    // Set PIVOT.GTIM.MvTyp.q.DetailQuality.OldData at value 1
    Logger::getLogger()->debug("%s set attribute %s at value 1", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonOldData.c_str());
    createIntegerElement(dpsDetailQuality, ConstantsMvCyclingCheck::KeyMessagePivotJsonOldData, 1); 

    // Set PIVOT.GTIM.MvTyp.q.Validity at value "questionable"
    Logger::getLogger()->debug("%s set attribute %s at value %s", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonValidity.c_str(), ConstantsMvCyclingCheck::ValueQuestionable.c_str());
    createStringElement(dpsQ, ConstantsMvCyclingCheck::KeyMessagePivotJsonValidity, ConstantsMvCyclingCheck::ValueQuestionable);
    
    // Set PIVOT.GTIM.MvTyp.q.Source at value "substituted"
    Logger::getLogger()->debug("%s set attribute %s at value %s", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonSource.c_str(), ConstantsMvCyclingCheck::ValueSubstituted.c_str());                    
    createStringElement(dpsQ, ConstantsMvCyclingCheck::KeyMessagePivotJsonSource, ConstantsMvCyclingCheck::ValueSubstituted);
}

/**
 * Set quality of timestamp
 * 
 * @param dpsRecGtIM : pointer of dictionary on PIVOT.GTIM.MvTyp
 * @param beforeLog : prefix of logs
*/
void ThreadCheckMv::setTimestampQuality(Datapoints *dpsRecGtIM, string beforeLog) {
    // Get PIVOT.GTIM.TmOrg
    Datapoints *dpsTmOrg = findDictElement(dpsRecGtIM, ConstantsMvCyclingCheck::KeyMessagePivotJsonTmOrg);
    if(dpsTmOrg == nullptr){
        Datapoint *createDpTmOrg = createDictElement(dpsRecGtIM, ConstantsMvCyclingCheck::KeyMessagePivotJsonTmOrg);
        dpsTmOrg = createDpTmOrg->getData().getDpVec();
    }

    // Set PIVOT.GTIM.TmOrg.StVal at value "substituted"
    Logger::getLogger()->debug("%s set attribute %s.StVal at value %s", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonTmOrg.c_str(), ConstantsMvCyclingCheck::ValueSubstituted.c_str());
    createStringElement(dpsTmOrg, ConstantsMvCyclingCheck::KeyMessagePivotJsonStVal, ConstantsMvCyclingCheck::ValueSubstituted);
    
    // Get PIVOT.GTIM.TmValidity
    Datapoints *dpsTmValidity = findDictElement(dpsRecGtIM, ConstantsMvCyclingCheck::KeyMessagePivotJsonTmValidity);
    if(dpsTmValidity == nullptr){
        Datapoint *createDpTmValidity = createDictElement(dpsRecGtIM, ConstantsMvCyclingCheck::KeyMessagePivotJsonTmValidity);
        dpsTmValidity = createDpTmValidity->getData().getDpVec();
    }

    // Set PIVOT.GTIM.TmValidity.StVal at value "valid"
    Logger::getLogger()->debug("%s set attribute %s.StVal at value %s", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonTmValidity.c_str(), ConstantsMvCyclingCheck::ValueValid.c_str());
    createStringElement(dpsTmValidity, ConstantsMvCyclingCheck::KeyMessagePivotJsonStVal, ConstantsMvCyclingCheck::ValueValid);
}

/**
 * Set a timestamp when the value measured is detected no renewal
 * 
 * @param dpsMvTyp : pointer of dictionary on PIVOT.GTIM.MvTyp
 * @param beforeLog : prefix of logs
 * @param currentTimestamp : timestamp current (ms)
*/
void ThreadCheckMv::setTimestampMVNoRenewal(Datapoints *dpsMvTyp, string beforeLog, long currentTimestamp) {
    // Get PIVOT.GTIM.MvTyp.t
    Datapoints *dpTimestamps = findDictElement(dpsMvTyp, ConstantsMvCyclingCheck::KeyMessagePivotJsonTs);
    if(dpTimestamps == nullptr){
        Datapoint *createDpTs = createDictElement(dpsMvTyp, ConstantsMvCyclingCheck::KeyMessagePivotJsonTs);
        dpTimestamps = createDpTs->getData().getDpVec();
    }

    std::pair<long, long> pTs = UtilityPivot::fromTimestamp(currentTimestamp);
    createIntegerElement(dpTimestamps, ConstantsMvCyclingCheck::KeyMessagePivotJsonSecondSinceEpoch, pTs.first);
    Logger::getLogger()->debug("%s set attribute %s at value %d", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonSecondSinceEpoch.c_str(), pTs.first);

    createIntegerElement(dpTimestamps, ConstantsMvCyclingCheck::KeyMessagePivotJsonFractionOfSecond, pTs.second);
    Logger::getLogger()->debug("%s set attribute %s at value %d", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonFractionOfSecond.c_str(), pTs.second);    
}
 
/**
 * The method reads the non-renewal measured value
*/
vector<Reading*> ThreadCheckMv::getReadingsNoRenewal() {
    return m_vecReadingNoRenewal;
}
 
/**
 * The method clear the received non-renewal measured value
*/
void ThreadCheckMv::clearReadingsNoRenewal() {
    m_vecReadingNoRenewal.clear();
}

/**
 * Get integer of check period
 * 
 * @return int value on check period
*/
int ThreadCheckMv::getCheckPeriod() { 
    return m_checkPeriod; 
}

/**
 * Get object StoreReceiveM
 * 
 * @return Object store
*/
StoreReceiveMv& ThreadCheckMv::getStore() { 
    return m_store; 
}

/**
 * Get booleen status of thread
 * 
 * @return booleen on state thread
*/
bool ThreadCheckMv::getStarted() { 
    return m_started; 
}