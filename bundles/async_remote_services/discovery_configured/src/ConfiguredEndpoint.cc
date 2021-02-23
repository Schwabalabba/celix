/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 *  KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <ConfiguredEndpoint.h>

namespace celix::rsa {

constexpr const char* ENDPOINT_IDENTIFIER = "endpoint.id";
constexpr const char* ENDPOINT_IMPORTED = "service.imported";
constexpr const char* ENDPOINT_IMPORT_CONFIGS = "service.imported.configs";
constexpr const char* ENDPOINT_EXPORTS = "service.exported.interfaces";
constexpr const char* ENDPOINT_OBJECTCLASS = "endpoint.objectClass";
constexpr const char* ENDPOINT_SCOPE = "endpoint.scope";
constexpr const char* ENDPOINT_TOPIC = "endpoint.topic";

celix::dm::Properties convertEndpointPropertiesToCelix(const ConfiguredEndpointProperties& endpointProperties) {

    return celix::dm::Properties{{"service.imported", std::to_string(endpointProperties.isImported()).c_str()},
                                 {"service.exported.interfaces", endpointProperties.getExports()},
                                 {"endpoint.id", endpointProperties.getId()}};
}

ConfiguredEndpointProperties convertCelixPropertiesToEndpoint(const celix::dm::Properties& celixProperties) {

    auto endpointId = celixProperties.at("endpoint.id");
    auto exports = celixProperties.at("service.exported.interfaces");
    auto imported = celixProperties.at("service.imported");
    return ConfiguredEndpointProperties{endpointId,
                                        (imported == "true"),
                                        {}, exports, {}, "", ""};
}

bool isValidEndpointJson(const rapidjson::Value& endpointJson) {

    return (endpointJson.HasMember(ENDPOINT_IDENTIFIER)
         && endpointJson.HasMember(ENDPOINT_IMPORTED)
         && endpointJson.HasMember(ENDPOINT_IMPORT_CONFIGS)
         && endpointJson.HasMember(ENDPOINT_EXPORTS)
         && endpointJson.HasMember(ENDPOINT_OBJECTCLASS)
         && endpointJson.HasMember(ENDPOINT_SCOPE)
         && endpointJson.HasMember(ENDPOINT_TOPIC));
}

std::vector<std::string> parseJSONStringArray(const rapidjson::Value& jsonArray) {

    std::vector<std::string> resultVec{};
    if(jsonArray.IsArray()) {
        resultVec.reserve(jsonArray.Size());
        for(auto& element : jsonArray.GetArray()) {
            if(element.IsString()) {
                resultVec.emplace_back(element.GetString());
            }
        }
    }
    return resultVec;
}

ConfiguredEndpoint::ConfiguredEndpoint(const rapidjson::Value& endpointJson) :
        Endpoint(celix::dm::Properties{}),
        _configuredProperties{} {

    if (isValidEndpointJson(endpointJson)) {

        _configuredProperties = {endpointJson[ENDPOINT_IDENTIFIER].GetString(),
                                 endpointJson[ENDPOINT_IMPORTED].GetBool(),
                                 parseJSONStringArray(endpointJson[ENDPOINT_IMPORT_CONFIGS]),
                                 endpointJson[ENDPOINT_EXPORTS].GetString(),
                                 parseJSONStringArray(endpointJson[ENDPOINT_OBJECTCLASS]),
                                 endpointJson[ENDPOINT_SCOPE].GetString(),
                                 endpointJson[ENDPOINT_TOPIC].GetString()};

        _celixProperties = convertEndpointPropertiesToCelix(*_configuredProperties);
    }
}

const ConfiguredEndpointProperties& ConfiguredEndpoint::getConfiguredProperties() const {

    return *_configuredProperties;
}

} // end namespace celix::rsa.
