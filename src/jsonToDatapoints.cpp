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
#include <jsonToDatapoints.h>

using namespace std;
using namespace rapidjson;
using namespace DatapointUtility; 

/**
 * Parsing a Json string
 * 
 * @param json : string json 
 * @return vector of datapoints
*/
Datapoints *JsonToDatapoints::parseJson(const string& json) {
	
	Document document;

	const auto& parseResult = document.Parse(json.c_str());
    if (parseResult.HasParseError()) {
        Logger::getLogger()->fatal("Parsing error %d (%s).", parseResult.GetParseError(), json.c_str());
        printf("Parsing error %d (%s).", parseResult.GetParseError(), json.c_str());
        return nullptr;
    }

    if (!document.IsObject()) {
        return nullptr;
    }
	return recursivJson(document);
}

/**
 * recursive method to convert a JSON string to a datapoint 
 * 
 * @param document : object rapidjon 
 * @return vector of datapoints
*/
Datapoints *JsonToDatapoints::recursivJson(const Value& document) {
	Datapoints *p = new Datapoints;

	for (Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
	{        
		if (itr->value.IsObject()) {
			Datapoints *vec = recursivJson(itr->value);
			DatapointValue d(vec, true);
			p->push_back(new Datapoint(itr->name.GetString(), d));
		}
		else if (itr->value.IsString()) {
			DatapointValue d(itr->value.GetString());
			p->push_back(new Datapoint(itr->name.GetString(), d));
		}
		else if (itr->value.IsDouble()) {
			DatapointValue d(itr->value.GetDouble());
			p->push_back(new Datapoint(itr->name.GetString(), d));
		}
		else if (itr->value.IsNumber() && !itr->value.IsDouble()) {
			DatapointValue d((long)itr->value.GetInt());
			p->push_back(new Datapoint(itr->name.GetString(), d));
		}
	}

	return p;
}