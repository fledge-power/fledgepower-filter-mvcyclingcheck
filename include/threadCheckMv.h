#ifndef INCLUDE_THREAD_CHECK_MV_H_
#define INCLUDE_THREAD_CHECK_MV_H_

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
#include <mutex>
#include <thread>
#include <reading.h>
#include <storeReceiveMv.h>
#include <datapointUtility.h>

class ThreadCheckMv {

public:  
    ThreadCheckMv();
    ~ThreadCheckMv();

    void start();
    void stop() ;
    void runCheckMv();

    void receiveMv(const std::string& assetName, Datapoint *dp, const std::string& id);
    void clearReadingsNoRenewal();
    std::vector<Reading*> getReadingsNoRenewal();

    void            setCheckPeriod(int checkPeriod);
    int             getCheckPeriod();
    StoreReceiveMv& getStore();
    bool            getStarted();

private:
    void setQuality(DatapointUtility::Datapoints *dpsQ, std::string beforeLog);
    void setTimestampQuality(DatapointUtility::Datapoints *dpsRecGtIM, std::string beforeLog);
    void setTimestampMVNoRenewal(DatapointUtility::Datapoints *dpsMvTyp, std::string beforeLog, long currentTimestamp);

    void deleteMv(const std::string& id);

    std::thread     *m_checkMv = nullptr;   
    StoreReceiveMv  m_store;

    int             m_checkPeriod;
    int             m_timeoutLoopThread;
    bool            m_started;
    std::mutex      m_mutex;

    std::vector<Reading*> m_vecReadingNoRenewal;
};

#endif  // INCLUDE_THREAD_CHECK_MV_H_
