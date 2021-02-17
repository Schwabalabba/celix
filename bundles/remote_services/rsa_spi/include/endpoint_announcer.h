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
#pragma once

#include <endpoint.h>
#include <celix/dm/Properties.h>

namespace celix::rsa {

/**
 * Interface defining functions for all compatible endpoint announcer classes.
 */
class IEndpointAnnouncer {
public:

    /**
     * Defaulted constructor.
     */
    IEndpointAnnouncer() = default;

    /**
     * Defaulted virtual destructor.
     */
    virtual ~IEndpointAnnouncer() = default;

    /**
     * Task the endpoint announcer to make the given endpoint visible for discovery by other managers/ frameworks.
     * @param endpoint The endpoint pointer in question.
     * @param properties The celix properties concerning the endpoint.
     */
    virtual void announceEndpoint(IEndpoint *endpoint, celix::dm::Properties&& properties) = 0;

    /**
     * Task the endpoint announcer to remove the discoverability of a given endpoint.
     * @param endpoint The endpoint pointer in question.
     * @param properties The celix properties concerning the endpoint, used for lookup.
     */
    virtual void revokeEndpoint(IEndpoint *endpoint, celix::dm::Properties&& properties) = 0;
};

} // end namespace celix::rsa.