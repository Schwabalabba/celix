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

#include <celix_api.h>
#include <string>

struct IHardcodedService {
    virtual ~IHardcodedService() = default;

    virtual int add(int a, int b) noexcept = 0;
    virtual int subtract(int a, int b) noexcept = 0;
    virtual std::string toString(int a) = 0;
};

struct HardcodedService final : public IHardcodedService {
    ~HardcodedService() final = default;

    int add(int a, int b) noexcept final {
        return a + b;
    }

    int subtract(int a, int b) noexcept final {
        return a - b;
    }

    std::string toString(int a) noexcept final {
        return std::to_string(a);
    }
};

struct ExportedHardcodedService final : public IHardcodedService {
    ~ExportedHardcodedService() final = default;

    int add(int a, int b) noexcept final {
        return a + b;
    }

    int subtract(int a, int b) noexcept final {
        return a - b;
    }

    std::string toString(int a) noexcept final {
        return std::to_string(a);
    }
};

struct ImportedHardcodedService final : public IHardcodedService {
    ~ImportedHardcodedService() final = default;

    int add(int a, int b) noexcept final {
        return a + b;
    }

    int subtract(int a, int b) noexcept final {
        return a - b;
    }

    std::string toString(int a) noexcept final {
        return std::to_string(a);
    }
};

class ExampleActivator {
public:
    explicit ExampleActivator([[maybe_unused]] std::shared_ptr<celix::dm::DependencyManager> mng) {

    }

    ExampleActivator(const ExampleActivator &) = delete;
    ExampleActivator &operator=(const ExampleActivator &) = delete;
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(ExampleActivator)