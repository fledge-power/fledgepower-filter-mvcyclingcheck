#ifndef INCLUDE_JSON_TO_DATAPOINT_H_
#define INCLUDE_JSON_TO_DATAPOINT_H_
/*
 * Json to Datapoints
 *
 * Copyright (c) 2020, RTE (https://www.rte-france.com)
 *
 * Released under the Apache 2.0 Licence
 *
 * Author: Yannick Marchetaux
 * 
 */

#include <datapointUtility.h>
#include <rapidjson/document.h>

namespace JsonToDatapoints {

    // Parsing JSON to datapoints
    DatapointUtility::Datapoints *parseJson(const std::string& json);
    DatapointUtility::Datapoints *recursivJson(const rapidjson::Value& document);
};

#endif  // INCLUDE_JSON_TO_DATAPOINT_H_