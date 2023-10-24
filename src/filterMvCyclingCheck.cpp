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
#include <string>

#include <filterMvCyclingCheck.h>
#include <datapointUtility.h>
#include <constantsMvCyclingCheck.h>

using namespace std;
using namespace DatapointUtility;

/**
 * Constructor for the LogFilter.
 *
 * We call the constructor of the base class and handle the initial
 * configuration of the filter.
 *
 * @param    filterName      The name of the filter
 * @param    filterConfig    The configuration category for this filter
 * @param    outHandle       The handle of the next filter in the chain
 * @param    output          A function pointer to call to output data to the next filter
 */
FilterMvCyclingCheck::FilterMvCyclingCheck(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM output) :
                                FledgeFilter(filterName, filterConfig, outHandle, output)
{        
    this->m_threadCheckMv.start();
}

/**
 * The actual filtering code
 *
 * @param readingSet The reading data to filter
 */
void FilterMvCyclingCheck::ingest(READINGSET *readingSet) 
{
    lock_guard<mutex> guard(m_configMutex);
	 
    // Filter enable, process the readings 
    if (!isEnabled()) {
        (*m_func)(m_data, readingSet);
        return ;
    }

    // Just get all the readings in the readingset
    const Readings& readings = readingSet->getAllReadings();
    for (auto reading = readings.cbegin(); reading != readings.cend(); reading++) {
        
        // Get datapoints on readings
        Datapoints& dataPoints = (*reading)->getReadingData();
        string assetName = (*reading)->getAssetName();  

        string beforeLog = ConstantsMvCyclingCheck::NamePlugin + " - " + assetName + " - ingest : ";
        
        Datapoints *dpPivotTM = findDictElement(&dataPoints, ConstantsMvCyclingCheck::KeyMessagePivotJsonRoot);
        if (dpPivotTM == nullptr){
            Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonRoot.c_str());
            continue;
        }
        
        Datapoints *dpGtIM = findDictElement(dpPivotTM, ConstantsMvCyclingCheck::KeyMessagePivotJsonGt);
        if (dpGtIM == nullptr){
            Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonGt.c_str());
            continue;
        }
               
        Datapoints *dpMvTyp = findDictElement(dpGtIM, ConstantsMvCyclingCheck::KeyMessagePivotJsonCdcMv);
        if(dpMvTyp == nullptr){
            Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonCdcMv.c_str());
            continue;
        }

        string dpID = findStringElement(dpGtIM, ConstantsMvCyclingCheck::KeyMessagePivotJsonId);
        if(dpID == ""){
            Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonId.c_str());
            continue;
        }

        Datapoints *dpCause = findDictElement(dpGtIM, ConstantsMvCyclingCheck::KeyMessagePivotJsonCot);
        if(dpCause == nullptr){
            Logger::getLogger()->debug("%s Missing %s attribute, it is ignored", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonCot.c_str());
            continue;
        }

        DatapointValue *valueStVal = findValueElement(dpCause, ConstantsMvCyclingCheck::KeyMessagePivotJsonStVal);
        if(valueStVal == nullptr){
            Logger::getLogger()->debug("%s Missing Cause %s attribute, it is ignored", beforeLog.c_str(), ConstantsMvCyclingCheck::KeyMessagePivotJsonStVal.c_str());
            continue;
        }

        long causeStVal = 0;
        if (valueStVal->getType() == DatapointValue::T_FLOAT) {
            causeStVal = (long)valueStVal->toDouble();
        }
        else if (valueStVal->getType() == DatapointValue::T_INTEGER) {
            causeStVal = valueStVal->toInt();
        }

        if(causeStVal != 1) {  // the measured value " TM " is configured not cyclic  
            Logger::getLogger()->debug("%s Cause %lu!=1, it is ignored", beforeLog.c_str(), causeStVal);
            continue;
        }

        Datapoint *dp = findDatapointElement(&dataPoints, ConstantsMvCyclingCheck::KeyMessagePivotJsonRoot); 
        m_threadCheckMv.receiveMv(assetName, dp, dpID);

        Logger::getLogger()->debug("%s Receiv and store PIVOTTM %s", beforeLog.c_str(), dp->toJSONProperty().c_str());

    }
    vector<Reading*> noRenewalReading = m_threadCheckMv.getReadingsNoRenewal();
    readingSet->append(noRenewalReading);
    
    (*m_func)(m_data, readingSet);

    m_threadCheckMv.clearReadingsNoRenewal();
}

/**
 * Reconfiguration entry point to the filter.
 *
 * This method runs holding the configMutex to prevent
 * ingest using the regex class that may be destroyed by this
 * call.
 *
 * Pass the configuration to the base FilterPlugin class and
 * then call the private method to handle the filter specific
 * configuration.
 *
 * @param newConfig  The JSON of the new configuration
 */
void FilterMvCyclingCheck::reconfigure(const std::string& newConfig) {
    lock_guard<mutex> guard(m_configMutex);
    setConfig(newConfig);
    
    ConfigCategory config("newConfig", newConfig);
    if (config.itemExists("check_period")) {
        m_threadCheckMv.setCheckPeriod(stoi(config.getValue("check_period").c_str())); 
    }
}

/**
 * Get object thread
 * 
 * @return object of thread 
*/
ThreadCheckMv& FilterMvCyclingCheck::getThread(){ 
    return m_threadCheckMv; 
}