
#ifndef _MANAGEMENT_SUBSCRIPTION_
#define _MANAGEMENT_SUBSCRIPTION_

//
// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
// 
//   http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.
//

// This source file was created by a code generator.
// Please do not edit.

#include "qpid/management/ManagementObject.h"

namespace qpid {
    namespace management {
        class ManagementAgent;
    }
}

namespace qmf {
namespace org {
namespace apache {
namespace qpid {
namespace broker {


class Subscription : public ::qpid::management::ManagementObject
{
  private:

    static std::string packageName;
    static std::string className;
    static uint8_t     md5Sum[MD5_LEN];


    // Properties
    ::qpid::management::ObjectId sessionRef;
    ::qpid::management::ObjectId queueRef;
    std::string name;
    bool browsing;
    bool acknowledged;
    bool exclusive;
    std::string creditMode;
    ::qpid::types::Variant::Map arguments;

    // Statistics


    // Per-Thread Statistics
    struct PerThreadStats {
        uint64_t  delivered;

    };

    struct PerThreadStats** perThreadStatsArray;

    inline struct PerThreadStats* getThreadStats() {
        int idx = getThreadIndex();
        struct PerThreadStats* threadStats = perThreadStatsArray[idx];
        if (threadStats == 0) {
            threadStats = new(PerThreadStats);
            perThreadStatsArray[idx] = threadStats;
            threadStats->delivered = 0;

        }
        return threadStats;
    }

    void aggregatePerThreadStats(struct PerThreadStats*) const;

  public:
    static void writeSchema(std::string& schema);
    void mapEncodeValues(::qpid::types::Variant::Map& map,
                         bool includeProperties=true,
                         bool includeStatistics=true);
    void mapDecodeValues(const ::qpid::types::Variant::Map& map);
    void doMethod(std::string&           methodName,
                  const ::qpid::types::Variant::Map& inMap,
                  ::qpid::types::Variant::Map& outMap,
                  const std::string& userId);
    std::string getKey() const;

    uint32_t writePropertiesSize() const;
    void readProperties(const std::string& buf);
    void writeProperties(std::string& buf) const;
    void writeStatistics(std::string& buf, bool skipHeaders = false);
    void doMethod(std::string& methodName,
                  const std::string& inBuf,
                  std::string& outBuf,
                  const std::string& userId);


    writeSchemaCall_t getWriteSchemaCall() { return writeSchema; }


    Subscription(::qpid::management::ManagementAgent* agent,
                            ::qpid::management::Manageable* coreObject, ::qpid::management::Manageable* _parent, const ::qpid::management::ObjectId& _queueRef, const std::string& _name, bool _browsing, bool _acknowledged, bool _exclusive, const ::qpid::types::Variant::Map& _arguments);
    ~Subscription();

    

    static void registerSelf(::qpid::management::ManagementAgent* agent);
    std::string& getPackageName() const { return packageName; }
    std::string& getClassName() const { return className; }
    uint8_t* getMd5Sum() const { return md5Sum; }

    // Method IDs

    // Accessor Methods
    inline void set_creditMode (const std::string& val) {
        ::qpid::management::Mutex::ScopedLock mutex(accessLock);
        creditMode = val;
        configChanged = true;
    }
    inline const std::string& get_creditMode() {
        ::qpid::management::Mutex::ScopedLock mutex(accessLock);
        return creditMode;
    }
    inline void inc_delivered (uint64_t by = 1) {
        getThreadStats()->delivered += by;
        instChanged = true;
    }
    inline void dec_delivered (uint64_t by = 1) {
        getThreadStats()->delivered -= by;
        instChanged = true;
    }

};

}}}}}

#endif  /*!_MANAGEMENT_SUBSCRIPTION_*/