project(LWMESH)

include_directories(nwk/inc)
include_directories(sys/inc)
include_directories(hal/atmega128rfa1/inc)
include_directories(phy/atmega128rfa1/inc)
include_directories(service/inc)
include_directories(${PROJECT_BINARY_DIR})

add_definitions("-DPHY_ATMEGA128RFA1 -DHAL_ATMEGA128RFA1"
                "-DPLATFORM_RCB128RFA1 -DF_CPU=16000000L ")
set(CMAKE_C_FLAGS "-Wall --std=gnu99 -O2 -fno-zero-initialized-in-bss -fdata-sections -ffunction-sections -fpack-struct -fshort-enums -funsigned-char -funsigned-bitfields -mmcu=atmega128rfa1")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -mmcu=atmega128rfa1 -Wl,-u,vfprintf")

option(NWK_ENABLE_ROUTING "enable lwmesh routing" OFF)
option(PHY_ENABLE_RANDOM_NUMBER_GENERATOR "enable hardware random number generator" ON)
set(LWMESH_NWK_BUFFERS_AMOUNT "3" CACHE STRING "lwmesh network buffers")
set(LWMESH_NWK_BUFFERS_AMOUNT "3" CACHE STRING "lwmesh network buffers")
set(LWMESH_NWK_MAX_ENDPOINTS_AMOUNT "3" CACHE STRING "lwmesh max endpoints")
set(LWMESH_NWK_DUPLICATE_REJECTION_TABLE_SIZE  "10" CACHE STRING "lwmesh duplicate rejection table size")
set(LWMESH_NWK_DUPLICATE_REJECTION_TTL "3000" CACHE STRING "lwmesh duplicate rejection table timeout (ms)")
set(LWMESH_NWK_ROUTE_TABLE_SIZE "100" CACHE STRING "lwmesh routing table size")
set(LWMESH_NWK_ROUTE_DEFAULT_SCORE "3" CACHE STRING "lwmesh route default score")
set(LWMESH_NWK_ACK_WAIT_TIME "300" CACHE STRING "lwmesh nwk ack wait time (ms)")

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h)

set(LWMESH_SRCS
  # Common to all MCUs
  nwk/src/nwk.c
  nwk/src/nwkDataReq.c
  nwk/src/nwkSecurity.c
  nwk/src/nwkFrame.c
#nwk/src/nwkGroup.c
  nwk/src/nwkRoute.c
#nwk/src/nwkRouteDiscovery.c
  nwk/src/nwkRx.c
  nwk/src/nwkTx.c
  sys/src/sys.c
  sys/src/sysTimer.c
  sys/src/sysEncrypt.c
  # Specific to ATmega128rfa1
  hal/atmega128rfa1/src/hal.c
  hal/atmega128rfa1/src/halTimer.c
  phy/atmega128rfa1/src/phy.c
#${DOF_FIRMWARE_SOURCE_DIR}/${STACK_PATH}/hal/drivers/atmega128rfa1/halSleep.c
#${DOF_FIRMWARE_SOURCE_DIR}/${STACK_PATH}/hal/drivers/atmega128rfa1/halUart.c
  # Specific to OTA
  service/src/otaClient.c
  service/src/otaServer.c
)
add_library(lwmesh STATIC ${LWMESH_SRCS})

