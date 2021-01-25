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
#include <mutex>
#include <memory_resource>
#include <ImportedServiceFactory.h>
#include <pubsub/api.h>
#include <celix/Deferred.h>
#include "IHardcodedService.h"
#include "HardcodedExampleSerializer.h"

struct ImportedHardcodedService final : public IHardcodedService {
    ImportedHardcodedService() = default;
    ~ImportedHardcodedService() final = default;

    ImportedHardcodedService(const ImportedHardcodedService&) = delete;
    ImportedHardcodedService(ImportedHardcodedService&&) = delete;
    ImportedHardcodedService& operator=(const ImportedHardcodedService&) = delete;
    ImportedHardcodedService& operator=(ImportedHardcodedService&&) = delete;

    celix::Promise<int> add(int a, int b) noexcept final {
        std::cout << "[ImportedHardcodedService] add" << std::endl;
//        std::this_thread::sleep_for(std::chrono::milliseconds(5000))
        std::unique_lock l(_m);
        AddArgs args{_idCounter++, a, b, {}};
        _publisher->send(_publisher->handle, 1, &args, nullptr);

        auto deferred = celix::Deferred<int>{};
        auto it = _intPromises.emplace(args.id, std::move(deferred));
        return it.first->second.getPromise();
    }

    celix::Promise<int> subtract(int a, int b) noexcept final {
        std::cout << "[ImportedHardcodedService] subtract" << std::endl;
        std::unique_lock l(_m);
        SubtractArgs args{_idCounter++, a, b, {}};
        _publisher->send(_publisher->handle, 2, &args, nullptr);

        auto deferred = celix::Deferred<int>{};
        auto it = _intPromises.emplace(args.id, std::move(deferred));
        return it.first->second.getPromise();
    }

    celix::Promise<std::string> toString(int a) noexcept final {
        std::cout << "[ImportedHardcodedService] toString" << std::endl;
        std::unique_lock l(_m);
        ToStringArgs args{_idCounter++, a, {}};
        _publisher->send(_publisher->handle, 3, &args, nullptr);

        auto deferred = celix::Deferred<std::string>{};
        auto it = _stringPromises.emplace(args.id, std::move(deferred));
        return it.first->second.getPromise();
    }

    void setPublisher(pubsub_publisher_t const * publisher, Properties&&) {
        std::cout << "[ImportedHardcodedService] setPublisher" << std::endl;
        _publisher = publisher;
    }

    int receiveMessage(const char *, unsigned int msgTypeId, void *msg, const celix_properties_t *) {
        std::cout << "[ImportedHardcodedService] receiveMessage" << std::endl;
        std::unique_lock l(_m);
        if(msgTypeId == 1) {
            auto response = static_cast<AddArgs*>(msg);

            if(!response->ret) {
                return 0;
            }

            auto deferred = _intPromises.find(response->id);
            if(deferred == end(_intPromises) || !response->ret) {
                return 1;
            }
            deferred->second.resolve(response->ret.value());
            _intPromises.erase(deferred);
        } else if(msgTypeId == 2) {
            auto response = static_cast<SubtractArgs*>(msg);

            if(!response->ret) {
                return 0;
            }

            auto deferred = _intPromises.find(response->id);
            if(deferred == end(_intPromises) || !response->ret) {
                return 1;
            }
            deferred->second.resolve(response->ret.value());
            _intPromises.erase(deferred);
        } else if(msgTypeId == 3) {
            auto response = static_cast<ToStringArgs*>(msg);

            if(!response->ret) {
                return 0;
            }

            auto deferred = _stringPromises.find(response->id);
            if(deferred == end(_stringPromises) || !response->ret) {
                return 1;
            }
            deferred->second.resolve(response->ret.value());
            _stringPromises.erase(deferred);
        }

        return 0;
    }

private:
    std::mutex _m{};
    pubsub_publisher_t const *_publisher{};
    std::pmr::unsynchronized_pool_resource _resource{};
    std::pmr::unordered_map<int, celix::Deferred<int>> _intPromises{&_resource};
    std::pmr::unordered_map<int, celix::Deferred<std::string>> _stringPromises{&_resource};
    static std::atomic<int> _idCounter;
};

std::atomic<int> ImportedHardcodedService::_idCounter = 0;

struct UsingHardcodedServiceService {

    void setService(IHardcodedService * svc, Properties&&) {
        std::cout << "[UsingHardcodedServiceService] setService" << std::endl;
        _svc = svc;
    }

    void start() {
        std::cout << "[UsingHardcodedServiceService] start" << std::endl;
        _t = std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            _svc->add(14, 123).thenAccept([](int val) {
                std::cout << "[UsingHardcodedServiceService] " << val << std::endl;
            });
        });
    }

    void stop() {
        if(_t.joinable()) {
            _t.join();
        }
    }

private:
    IHardcodedService *_svc{};
    std::thread _t{};
};

class ExampleActivator {
public:
    explicit ExampleActivator(std::shared_ptr<celix::dm::DependencyManager>& mng) {
        std::cout << "[ExampleActivator] ExampleActivator" << std::endl;
        _addArgsSerializer.emplace(mng);
        _subtractArgsSerializer.emplace(mng);
        _toStringSerializer.emplace(mng);

        mng->createComponent(std::make_unique<celix::async_rsa::DefaultImportedServiceFactory<IHardcodedService, ImportedHardcodedService>>(mng))
            .addInterface<celix::async_rsa::IImportedServiceFactory>("1.0.0", Properties{{"service.exported.interfaces", "IHardcodedService"}}).build();

        auto &usingCmp = mng->createComponent<UsingHardcodedServiceService>()
                .setCallbacks(nullptr, &UsingHardcodedServiceService::start, &UsingHardcodedServiceService::stop, nullptr);
        _usingSvc = &usingCmp.getInstance();
        usingCmp.createServiceDependency<IHardcodedService>().setCallbacks([this](IHardcodedService *svc, Properties&& props) {
            _usingSvc->setService(svc, std::forward<Properties>(props));
        })
        .setRequired(true)
        .build();

        usingCmp.build();
    }

    ~ExampleActivator() {
        std::cout << "[ExampleActivator] ~ExampleActivator" << std::endl;
        _addArgsSerializer.reset();
        _subtractArgsSerializer.reset();
        _toStringSerializer.reset();
    }

    ExampleActivator(const ExampleActivator &) = delete;
    ExampleActivator &operator=(const ExampleActivator &) = delete;

private:
    UsingHardcodedServiceService *_usingSvc{};

    std::optional<AddArgsSerializer> _addArgsSerializer{};
    std::optional<SubtractArgsSerializer> _subtractArgsSerializer{};
    std::optional<ToStringArgsSerializer> _toStringSerializer{};
};

CELIX_GEN_CXX_BUNDLE_ACTIVATOR(ExampleActivator)