
#ifndef _MANAGEMENT_TESTOBJECT_
#define _MANAGEMENT_TESTOBJECT_

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
namespace mgmt {
namespace test {


class TestObject : public ::qpid::management::ManagementObject
{
  private:

    static std::string packageName;
    static std::string className;
    static uint8_t     md5Sum[MD5_LEN];


    // Properties
    std::string string1;
    bool bool1;
    ::qpid::types::Variant::Map map1;

    // Statistics


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

    // Stub for getInstChanged.  There are no statistics in this class.
    bool getInstChanged() { return false; }
    bool hasInst() { return false; }


    TestObject(::qpid::management::ManagementAgent* agent,
                            ::qpid::management::Manageable* coreObject);
    ~TestObject();

    

    static void registerSelf(::qpid::management::ManagementAgent* agent);
    std::string& getPackageName() const { return packageName; }
    std::string& getClassName() const { return className; }
    uint8_t* getMd5Sum() const { return md5Sum; }

    // Method IDs

    // Accessor Methods
    inline void set_string1 (const std::string& val) {
        ::qpid::management::Mutex::ScopedLock mutex(accessLock);
        string1 = val;
        configChanged = true;
    }
    inline const std::string& get_string1() {
        ::qpid::management::Mutex::ScopedLock mutex(accessLock);
        return string1;
    }
    inline void set_bool1 (bool val) {
        ::qpid::management::Mutex::ScopedLock mutex(accessLock);
        bool1 = val;
        configChanged = true;
    }
    inline bool get_bool1() {
        ::qpid::management::Mutex::ScopedLock mutex(accessLock);
        return bool1;
    }
    inline void set_map1 (const ::qpid::types::Variant::Map& val) {
        ::qpid::management::Mutex::ScopedLock mutex(accessLock);
        map1 = val;
        configChanged = true;
    }
    inline const ::qpid::types::Variant::Map& get_map1() {
        ::qpid::management::Mutex::ScopedLock mutex(accessLock);
        return map1;
    }

};

}}}}}}}

#endif  /*!_MANAGEMENT_TESTOBJECT_*/
