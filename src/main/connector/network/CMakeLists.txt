# Build all files in this directory
SET(SOURCE
    NetworkConnector.cpp
)

SET(HEADERS
    NetworkConnector.h
)

find_package(Qt5Network REQUIRED)

source_group("Header Files" FILES ${HEADERS})
add_library(NetworkConnector ${SOURCE} ${HEADERS})
target_link_libraries(NetworkConnector RemoteControl Qt5::Core Qt5::Network)