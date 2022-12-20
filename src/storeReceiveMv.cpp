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
#include <storeReceiveMv.h>
#include <utilityPivot.h>

using namespace std;

/**
 * Erase the map by id allows to send on-renewal measured value once 
 * 
 * @param identifier : identifier to be deleted
*/
void StoreReceiveMv::eraseStoreMap(const string& identifier) {
    if(mapStoredData.find(identifier) != mapStoredData.end() && mapStoredData[identifier].dpStored != nullptr) {
        delete mapStoredData[identifier].dpStored;
    }
    mapStoredData.erase(identifier);
}

/**
 * This method store and duplicate the Datapoint,
 * in order to send the non-renewal measured value.
 * Runs on thread
 * 
 * @param assetName : name asset's
 * @param dp : pointer of datapoint
 * @param identifier : identifier of measured value
*/
void StoreReceiveMv::setOriginalDp(const string& assetName, Datapoint *dp, const string& identifier) {
    
    if(mapStoredData.find(identifier) != mapStoredData.end() && mapStoredData[identifier].dpStored != nullptr) {
        delete mapStoredData[identifier].dpStored;
    }

    DatapointValue newValue(dp->getData());
    Datapoint *newDatapoint = new Datapoint(dp->getName(), newValue);
    
    storeData dataToStore;
    dataToStore.dpStored = newDatapoint;
    dataToStore.timestamp = UtilityPivot::getCurrentTimestampMs();
    dataToStore.assetName = assetName;
    
    mapStoredData[identifier] = dataToStore;
}

/**
 * Getter the measured value map
 * 
 * @return map of <std::string, storeData>
*/
std::map<std::string, storeData> StoreReceiveMv::getMap() { 
    return mapStoredData; 
}
