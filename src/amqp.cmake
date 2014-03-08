#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#

# Optional AMQP1.0 support. Requires proton toolkit.

include(FindPkgConfig)

pkg_check_modules(PROTON libqpid-proton)

if (NOT PROTON_FOUND)
    # if pkg-config is absent or fails to find proton then use
    # PROTON_ROOT command line option or environment variable to locate
    # local installed proton build.
    if (NOT PROTON_ROOT)
        set (PROTON_ROOT "$ENV{PROTON_ROOT}")
    endif()
    if (PROTON_ROOT)
        find_package(proton PATHS ${PROTON_ROOT} NO_DEFAULT_PATH)

        if (proton_FOUND EQUAL 1)
            set(iFile "${PROTON_ROOT}/lib/proton.cmake/libqpid-proton.cmake")
            if(EXISTS ${iFile})
                include("${iFile}")
            else()
                message(FATAL_ERROR "PROTON_ROOT defined but file ${iFile} is missing")
            endif()
        else()
            message(FATAL_ERROR "Proton package files not found in ${PROTON_ROOT}")
        endif()
    endif()
endif()

set (amqp_default ${amqp_force})
set (minimum_version 0.5)
set (maximum_version 0.6)
if (PROTON_FOUND)
    if (PROTON_VERSION LESS ${minimum_version})
        message(STATUS "Qpid proton ${PROTON_VERSION} is too old, require ${minimum_version} - ${maximum_version}; amqp 1.0 support not enabled")
    else (PROTON_VERSION LESS ${minimum_version})
        if (PROTON_VERSION GREATER ${maximum_version})
            message(STATUS "Qpid proton ${PROTON_VERSION} is too new, require ${minimum_version} - ${maximum_version}; amqp 1.0 support not enabled")
        else (PROTON_VERSION GREATER ${maximum_version})
            message(STATUS "Qpid proton found, amqp 1.0 support enabled")
            set (amqp_default ON)
        endif (PROTON_VERSION GREATER ${maximum_version})
    endif (PROTON_VERSION LESS ${minimum_version})
else (PROTON_FOUND)
    message(STATUS "Qpid proton not found, amqp 1.0 support not enabled")
endif (PROTON_FOUND)

option(BUILD_AMQP "Build with support for AMQP 1.0" ${amqp_default})
if (BUILD_AMQP)

    if (NOT PROTON_FOUND)
      message(FATAL_ERROR "Qpid proton not found, required for amqp 1.0 support")
    endif (NOT PROTON_FOUND)

    set (amqp_SOURCES
         qpid/broker/amqp/Authorise.h
         qpid/broker/amqp/Authorise.cpp
         qpid/broker/amqp/BrokerContext.h
         qpid/broker/amqp/BrokerContext.cpp
         qpid/broker/amqp/Connection.h
         qpid/broker/amqp/Connection.cpp
         qpid/broker/amqp/DataReader.h
         qpid/broker/amqp/DataReader.cpp
         qpid/broker/amqp/Domain.h
         qpid/broker/amqp/Domain.cpp
         qpid/broker/amqp/Exception.h
         qpid/broker/amqp/Exception.cpp
         qpid/broker/amqp/Filter.h
         qpid/broker/amqp/Filter.cpp
         qpid/broker/amqp/Header.h
         qpid/broker/amqp/Header.cpp
         qpid/broker/amqp/Incoming.h
         qpid/broker/amqp/Incoming.cpp
         qpid/broker/amqp/Interconnect.h
         qpid/broker/amqp/Interconnect.cpp
         qpid/broker/amqp/Interconnects.h
         qpid/broker/amqp/Interconnects.cpp
         qpid/broker/amqp/ManagedConnection.h
         qpid/broker/amqp/ManagedConnection.cpp
         qpid/broker/amqp/ManagedSession.h
         qpid/broker/amqp/ManagedSession.cpp
         qpid/broker/amqp/ManagedIncomingLink.h
         qpid/broker/amqp/ManagedIncomingLink.cpp
         qpid/broker/amqp/ManagedOutgoingLink.h
         qpid/broker/amqp/ManagedOutgoingLink.cpp
         qpid/broker/amqp/Message.h
         qpid/broker/amqp/Message.cpp
         qpid/broker/amqp/NodePolicy.h
         qpid/broker/amqp/NodePolicy.cpp
         qpid/broker/amqp/NodeProperties.h
         qpid/broker/amqp/NodeProperties.cpp
         qpid/broker/amqp/Outgoing.h
         qpid/broker/amqp/Outgoing.cpp
         qpid/broker/amqp/ProtocolPlugin.cpp
         qpid/broker/amqp/Relay.h
         qpid/broker/amqp/Relay.cpp
         qpid/broker/amqp/Sasl.h
         qpid/broker/amqp/Sasl.cpp
         qpid/broker/amqp/SaslClient.h
         qpid/broker/amqp/SaslClient.cpp
         qpid/broker/amqp/Session.h
         qpid/broker/amqp/Session.cpp
         qpid/broker/amqp/Topic.h
         qpid/broker/amqp/Topic.cpp
         qpid/broker/amqp/Translation.h
         qpid/broker/amqp/Translation.cpp
        )

    include_directories(${PROTON_INCLUDE_DIRS})
    link_directories(${PROTON_LIBRARY_DIRS})

    add_library (amqp MODULE ${amqp_SOURCES})
    target_link_libraries (amqp qpidtypes qpidbroker qpidcommon ${PROTON_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY})
    set_target_properties (amqp PROPERTIES
                           PREFIX ""
                           LINK_FLAGS "${CATCH_UNDEFINED}"
                           COMPILE_DEFINITIONS _IN_QPID_BROKER)

    install (TARGETS amqp
             DESTINATION ${QPIDD_MODULE_DIR}
             COMPONENT ${QPID_COMPONENT_BROKER})

    set (amqpc_SOURCES
         qpid/messaging/amqp/AddressHelper.h
         qpid/messaging/amqp/AddressHelper.cpp
         qpid/messaging/amqp/ConnectionContext.h
         qpid/messaging/amqp/ConnectionContext.cpp
         qpid/messaging/amqp/ConnectionHandle.h
         qpid/messaging/amqp/ConnectionHandle.cpp
         qpid/messaging/amqp/DriverImpl.h
         qpid/messaging/amqp/DriverImpl.cpp
         qpid/messaging/amqp/ReceiverContext.h
         qpid/messaging/amqp/ReceiverContext.cpp
         qpid/messaging/amqp/ReceiverHandle.h
         qpid/messaging/amqp/ReceiverHandle.cpp
         qpid/messaging/amqp/Sasl.h
         qpid/messaging/amqp/Sasl.cpp
         qpid/messaging/amqp/SenderContext.h
         qpid/messaging/amqp/SenderContext.cpp
         qpid/messaging/amqp/SenderHandle.h
         qpid/messaging/amqp/SenderHandle.cpp
         qpid/messaging/amqp/SessionContext.h
         qpid/messaging/amqp/SessionContext.cpp
         qpid/messaging/amqp/SessionHandle.h
         qpid/messaging/amqp/SessionHandle.cpp
         qpid/messaging/amqp/TcpTransport.h
         qpid/messaging/amqp/TcpTransport.cpp
        )

    if (WIN32)
        set(proton_dll  "${PROTON_LIBRARY_DIRS}/${PROTON_LIBRARIES}.dll")
        set(proton_dlld "${PROTON_LIBRARY_DIRS}/${PROTON_LIBRARIES}d.dll")

        install (PROGRAMS  ${proton_dll}
                DESTINATION ${QPID_INSTALL_BINDIR}
                COMPONENT   ${QPID_COMPONENT_COMMON}
                CONFIGURATIONS Release|MinSizeRel|RelWithDebInfo)
        install (PROGRAMS  ${proton_dlld}
                DESTINATION ${QPID_INSTALL_BINDIR}
                COMPONENT   ${QPID_COMPONENT_COMMON}
                CONFIGURATIONS Debug)
    endif (WIN32)
else (BUILD_AMQP)
    # ensure that qpid build ignores proton
    UNSET( amqpc_SOURCES )
    UNSET( PROTON_LIBRARIES )
endif (BUILD_AMQP)
