#ifndef INCLUDE_STORE_RECEIVE_MV_H_
#define INCLUDE_STORE_RECEIVE_MV_H_

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
#include <map>
#include <datapoint.h>

struct storeData {
    Datapoint   *dpStored = nullptr;
    long        timestamp = 0;
    std::string assetName = "";
};

class StoreReceiveMv {

    public: 
        StoreReceiveMv() = default;

        void setOriginalDp(const std::string& assetName, Datapoint *dp, const std::string& identifier);
        void eraseStoreMap(const std::string& identifier);

        std::map<std::string, storeData> getMap();
        
    private:
        std::map<std::string, storeData> mapStoredData;
};
#endif  // INCLUDE_STORE_RECEIVE_MV_H_

