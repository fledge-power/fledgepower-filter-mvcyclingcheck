#ifndef INCLUDE_CONSTANTS_MV_CYCLING_CHECK_H_
#define INCLUDE_CONSTANTS_MV_CYCLING_CHECK_H_

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

#define FILTER_NAME "mvcyclingcheck"

namespace ConstantsMvCyclingCheck {

    const std::string KeyMessagePivotJsonRoot                   = "PIVOT";
    const std::string KeyMessagePivotJsonGt                     = "GTIM";
    const std::string KeyMessagePivotJsonId                     = "Identifier";
    const std::string KeyMessagePivotJsonCot                    = "Cause";
    const std::string KeyMessagePivotJsonStVal                  = "stVal";
    const std::string KeyMessagePivotJsonCdcMv                  = "MvTyp";
    const std::string KeyMessagePivotJsonSecondSinceEpoch       = "SecondSinceEpoch";
    const std::string KeyMessagePivotJsonFractionOfSecond       = "FractionOfSecond";
    const std::string KeyMessagePivotJsonTs                     = "t";
    const std::string KeyMessagePivotJsonQ                      = "q";
    const std::string KeyMessagePivotJsonOldData                = "oldData";
    const std::string KeyMessagePivotJsonDetailQuality          = "DetailQuality";
    const std::string KeyMessagePivotJsonValidity               = "Validity";
    const std::string KeyMessagePivotJsonSource                 = "Source";
    const std::string KeyMessagePivotJsonTmOrg                  = "TmOrg";
    const std::string KeyMessagePivotJsonTmValidity             = "TmValidity";
    const std::string ValueSubstituted                          = "substituted";
    const std::string ValueValid                                = "valid";
    const std::string ValueQuestionable                         = "questionable";
    const std::string NamePlugin                                = FILTER_NAME;
};

#endif  // INCLUDE_CONSTANTS_MV_CYCLING_CHECK_H_

