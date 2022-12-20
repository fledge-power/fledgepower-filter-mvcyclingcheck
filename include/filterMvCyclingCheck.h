#ifndef INCLUDE_FILTER_MV_CYCLING_CHECK_H_
#define INCLUDE_FILTER_MV_CYCLING_CHECK_H_

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
#include <config_category.h>
#include <filter.h>
#include <mutex>
#include <string>

#include <threadCheckMv.h>
#include <storeReceiveMv.h>

class FilterMvCyclingCheck  : public FledgeFilter
{
public:  
    FilterMvCyclingCheck(const std::string& filterName,
                        ConfigCategory& filterConfig,
                        OUTPUT_HANDLE *outHandle,
                        OUTPUT_STREAM output);

    void ingest(READINGSET *readingSet);
    void reconfigure(const std::string& newConfig);

    ThreadCheckMv& getThread();
    
private:
    std::mutex      m_configMutex;
    ThreadCheckMv   m_threadCheckMv;
};

#endif  // INCLUDE_FILTER_MV_CYCLING_CHECK_H_