INCLUDE_DIRECTORIES(./)
INCLUDE_DIRECTORIES(./thridpart)
LINK_DIRECTORIES(/usr/local/lib)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -W -static-libstdc++")  

configure_file (
	"${PROJECT_SOURCE_DIR}/src/Version.hpp.in"	
	"${PROJECT_SOURCE_DIR}/src/Version.hpp"	
)

configure_file (
	"${PROJECT_SOURCE_DIR}/rpm/aidp.spec.in"	
	"${PROJECT_SOURCE_DIR}/rpm/aidp.spec"	
)

configure_file (
	"${PROJECT_SOURCE_DIR}/rpm/build_rpm.in"	
	"${PROJECT_SOURCE_DIR}/rpm/build_rpm"	
	@ONLY
)

SET(AIDP_SRC aidp.cpp
	BootStrap.cpp	
	App.cpp	
	AdServer.cpp
	Http.cpp
	Http/HttpInterface.cpp
	Http/Server.cpp
	Http/Index.cpp
	McProcessor.cpp
	HeadProcessor.cpp
	Aims.cpp
	Aims/Kafka/ConsumerOut.cpp
	Timer.cpp
)
ADD_EXECUTABLE(aidp ${AIDP_SRC})
TARGET_LINK_LIBRARIES(aidp libadbase.a 
	libadbase_kafka.a
	librdkafka.a 
	libevent.a
	pthread
	rt
	z
	)
INSTALL(TARGETS aidp RUNTIME DESTINATION adinf/aidp/bin)