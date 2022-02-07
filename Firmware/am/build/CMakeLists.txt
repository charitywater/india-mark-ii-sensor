# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~
# FILE:
#     Build/CMakeLists.txt
#
# Top-Level CMake file for AM build
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+~


INCLUDE(CMakeForceCompiler)

# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
message(STATUS, "CMake Version: ${CMAKE_VERSION} ")

# CURRENT DIRECTORY
SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})
SET(CMAKE_SOURCE_DIR ${ProjDirPath}/../)

# Build Timestamp
string(TIMESTAMP  BUILD_TIMESTAMP_UTC  %Y-%m-%dT%H:%M:%SZ  UTC)

# Git Branch Name
execute_process(
    COMMAND bash "-c" "/usr/bin/git symbolic-ref -q --short HEAD || /usr/bin/git describe --tags --exact-match"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Git Abbreviated Commit Hash
execute_process(
  COMMAND /usr/bin/git log --max-count=1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Build version header from template
configure_file(
    ${CMAKE_SOURCE_DIR}/build/version-git-info.h.in
    ${CMAKE_SOURCE_DIR}/inc/version-git-info.h
)


# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# LINK FILES
SET(CMAKE_EXE_LINKER_FLAGS_GENERIC "-T ${ProjDirPath}/../STM32L4R5VITx_FLASH.ld")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${CMAKE_EXE_LINKER_FLAGS_GENERIC}")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${CMAKE_EXE_LINKER_FLAGS_GENERIC}")

# ASM FLAGS
SET(CMAKE_ASM_FLAGS_GENERIC "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fmessage-length=0 -ffunction-sections -Wall -std=gnu99")
SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_ASM_FLAGS_RELEASE} ${CMAKE_ASM_FLAGS_GENERIC}")
SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_ASM_FLAGS_DEBUG} ${CMAKE_ASM_FLAGS_GENERIC}")

# C FLAGS
SET(CMAKE_C_FLAGS_GENERIC "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fmessage-length=0 -ffunction-sections -Wall -std=gnu99")
SET(CMAKE_C_FLAGS_GENERIC "${CMAKE_C_FLAGS_GENERIC} '-D__weak=__attribute__((weak))'")
SET(CMAKE_C_FLAGS_GENERIC "${CMAKE_C_FLAGS_GENERIC} '-D__packed=__attribute__((__packed__))'")
SET(CMAKE_C_FLAGS_GENERIC "${CMAKE_C_FLAGS_GENERIC} -DSTM32L4R5xx")
SET(CMAKE_C_FLAGS_GENERIC "${CMAKE_C_FLAGS_GENERIC} -DUSE_HAL_DRIVER")
SET(CMAKE_C_FLAGS_GENERIC "${CMAKE_C_FLAGS_GENERIC} -DAM_BUILD")

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${CMAKE_C_FLAGS_GENERIC}")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${CMAKE_C_FLAGS_GENERIC}")

# LD FLAGS
SET(CMAKE_EXE_LINKER_FLAGS_GENERIC "-Xlinker --gc-sections -specs=nosys.specs -specs=nano.specs")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${CMAKE_EXE_LINKER_FLAGS_GENERIC}")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${CMAKE_EXE_LINKER_FLAGS_GENERIC}")


# INCLUDE_DIRECTORIES
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/inc)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/protos)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/configuration)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/abstractions)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/abstractions/platform)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/abstractions/platform/freertos)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/abstractions/platform/freertos/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/abstractions/platform/freertos/include/platform)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/abstractions/platform/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/abstractions/platform/include/platform)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/abstractions/platform/include/types)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/include/private)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/include/types)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/taskpool)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/include/types)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src/private)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/serializer)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/serializer/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/serializer/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/serializer/src/json)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/FreeRTOS)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/include/private)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/pkcs11/portable/st/stm32l4r5_nucleo)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/pkcs11/mbedtls)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/secure_sockets)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/secure_sockets/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/secure_sockets/portable/st/stm32l4r5_nucleo)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/crypto)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/crypto/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/crypto/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/pkcs11)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/pkcs11/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/pkcs11/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/tls)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/tls/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/tls/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/utils)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/utils/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/utils/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/api)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/ipv4)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/include/lwip)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/include/netif)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/include/netif/ppp)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/apps/http)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/system)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/system/arch)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/system/OS)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mcu_vendor/st/stm32l4r5_nucleo/BSP/Components/Common)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mcu_vendor/st/stm32l4r5_nucleo/BSP/STM32L4xx_Nucleo_144)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mcu_vendor/st/stm32l4r5_nucleo/BSP/Components/vl53l0x)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mcu_vendor/st/stm32l4r5_nucleo/CMSIS/Include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mcu_vendor/st/stm32l4r5_nucleo/CMSIS/Device/ST/STM32L4xx/Include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mcu_vendor/st/stm32l4r5_nucleo/STM32L4xx_HAL_Driver/Src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/include/mbedtls)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/utils)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atcacert)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/crypto)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/hal)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/host)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/jwt)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/mbedtls)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/tng)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/third_party/pkcs11)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/FreeRTOS/portable/MemMang)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/FreeRTOS)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/FreeRTOS/portable/GCC)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/FreeRTOS/portable/GCC/ARM_CM4F)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Inc)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Inc/Legacy)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/CMSIS/Device/ST/STM32L4xx)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/CMSIS/Device/ST/STM32L4xx/Include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/drivers/CMSIS/Include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../shared/asp/inc)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../shared/nvm/inc)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/application)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/device-drivers)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/handlers)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/peripheral-drivers)


# ADD_EXECUTABLE
ADD_EXECUTABLE(am
    "${CMAKE_SOURCE_DIR}/protos/messages.pb.c"
    "${CMAKE_SOURCE_DIR}/protos/pb_common.c"
    "${CMAKE_SOURCE_DIR}/protos/pb_decode.c"
    "${CMAKE_SOURCE_DIR}/protos/pb_encode.c"
    "${CMAKE_SOURCE_DIR}/startup/startup_stm32l4r5xx.s"
    "${CMAKE_SOURCE_DIR}/lib/abstractions/platform/freertos/iot_metrics.c"
    "${CMAKE_SOURCE_DIR}/lib/abstractions/platform/freertos/iot_network_freertos.c"
    "${CMAKE_SOURCE_DIR}/lib/abstractions/platform/freertos/iot_clock_freertos.c"
    "${CMAKE_SOURCE_DIR}/lib/abstractions/platform/freertos/iot_threads_freertos.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/tls/src/iot_tls.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/pkcs11/src/iot_pkcs11.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/crypto/src/iot_crypto.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/utils/src/iot_system_init.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/freeRtosPlus/utils/src/iot_pki_utils.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/api/tcpip.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/api/netbuf.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/api/api_lib.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/api/api_msg.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/api/err.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/api/sockets.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/api/netdb.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/ip.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/init.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/def.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/dns.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/inet_chksum.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/mem.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/memp.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/netif.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/pbuf.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/raw.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/sys.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/tcp.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/tcp_in.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/tcp_out.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/timeouts.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/udp.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/ipv4/ip4.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/ipv4/icmp.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/core/ipv4/ip4_addr.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/auth.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/fsm.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/ipcp.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/lcp.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/magic.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/ppp.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/pppapi.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/pppos.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/upap.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/utils.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/netif/ppp/vj.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/apps/http/http_client.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/src/apps/http/httpd.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/LwIP/system/OS/sys_arch.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mcu_vendor/st/stm32l4r5_nucleo/BSP/STM32L4xx_Nucleo_144/stm32l4xx_nucleo_144.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/aes.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/aesni.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/arc4.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/asn1parse.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/asn1write.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/base64.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/bignum.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/blowfish.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/camellia.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ccm.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/certs.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/cipher.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/cipher_wrap.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/cmac.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ctr_drbg.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/debug.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/des.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/dhm.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ecdh.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ecdsa.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ecjpake.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ecp.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ecp_curves.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/entropy.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/entropy_poll.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/error.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/gcm.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/havege.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/hmac_drbg.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/md.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/md2.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/md4.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/md5.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/md_wrap.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/memory_buffer_alloc.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/net_sockets.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/oid.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/padlock.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/pem.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/pk.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/pk_wrap.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/pkcs12.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/pkcs5.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/pkparse.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/pkwrite.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/platform.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/platform_util.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ripemd160.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/rsa.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/rsa_internal.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/sha1.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/sha256.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/sha512.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ssl_cache.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ssl_ciphersuites.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ssl_cli.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ssl_cookie.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ssl_srv.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ssl_ticket.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/ssl_tls.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/threading.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/timing.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/version.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/version_features.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/x509.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/x509_create.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/x509_crl.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/x509_crt.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/x509_csr.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/x509write_crt.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/x509write_csr.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/library/xtea.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/mbedtls/utils/mbedtls_utils.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atcacert/atcacert_client.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atcacert/atcacert_date.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atcacert/atcacert_def.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atcacert/atcacert_der.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atcacert/atcacert_host_hw.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atcacert/atcacert_host_sw.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atcacert/atcacert_pem.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atca_cfgs.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atca_command.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atca_device.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atca_execution.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/atca_iface.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_aes_cbc.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_aes_cmac.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_aes_ctr.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_aes_gcm.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_aes.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_checkmac.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_counter.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_derivekey.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_ecdh.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_gendig.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_genkey.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_hmac.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_info.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_kdf.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_lock.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_mac.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_nonce.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_privwrite.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_random.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_read.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_secureboot.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_selftest.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_sha.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_sign.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_updateextra.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_verify.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic_write.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_basic.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/basic/atca_helpers.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/crypto/atca_crypto_sw_ecdsa.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/crypto/atca_crypto_sw_rand.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/crypto/atca_crypto_sw_sha1.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/crypto/atca_crypto_sw_sha2.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/crypto/hashes/sha1_routines.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/crypto/hashes/sha2_routines.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/hal/atca_hal.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/hal/stm_i2c_ata.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/host/atca_host.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/jwt/atca_jwt.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/mbedtls/atca_mbedtls_ecdh.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/mbedtls/atca_mbedtls_wrap.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_main.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_digest.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_attrib.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_cert.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_config.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_debug.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_find.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_info.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_init.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_key.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_mech.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_object.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_os.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_session.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_signature.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_slot.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_token.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/pkcs11/pkcs11_util.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/tng/tng_root_cert.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/tng/tngtls_cert_def_1_signer.c"
    "${CMAKE_SOURCE_DIR}/lib/third_party/microchip/lib/tng/tngtls_cert_def_3_device.c"
    "${CMAKE_SOURCE_DIR}/lib/pkcs11/secure_element/iot_pkcs11_secure_element.c"
    "${CMAKE_SOURCE_DIR}/lib/pkcs11/portable/st/stm32l4r5_nucleo/iot_pkcs11_pal.c"
    "${CMAKE_SOURCE_DIR}/lib/secure_sockets/portable/st/stm32l4r5_nucleo/iot_secure_sockets.c"
    "${CMAKE_SOURCE_DIR}/lib/pkcs11/portable/st/stm32l4r5_nucleo/iot_pkcs11_pal.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/logging/iot_logging_task_dynamic_buffers.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/logging/iot_logging.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/taskpool/iot_taskpool_static_memory.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/taskpool/iot_taskpool.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/iot_device_metrics.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/iot_init.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/common/iot_static_memory_common.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src/iot_mqtt_agent.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src/iot_mqtt_api.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src/iot_mqtt_network.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src/iot_mqtt_operation.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src/iot_mqtt_serialize.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src/iot_mqtt_static_memory.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src/iot_mqtt_subscription.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/mqtt/src/iot_mqtt_validate.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/serializer/src/json/iot_json_utils.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/serializer/src/json/iot_serializer_static_memory.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/serializer/src/json/iot_serializer_json_decoder.c"
    "${CMAKE_SOURCE_DIR}/lib/c_sdk/standard/serializer/src/json/iot_serializer_json_encoder.c"
    "${CMAKE_SOURCE_DIR}/lib/FreeRTOS/portable/MemMang/heap_5.c"
    "${CMAKE_SOURCE_DIR}/lib/FreeRTOS/portable/GCC/ARM_CM4F/port.c"
    "${CMAKE_SOURCE_DIR}/lib/FreeRTOS/event_groups.c"
    "${CMAKE_SOURCE_DIR}/lib/FreeRTOS/list.c"
    "${CMAKE_SOURCE_DIR}/lib/FreeRTOS/queue.c"
    "${CMAKE_SOURCE_DIR}/lib/FreeRTOS/stream_buffer.c"
    "${CMAKE_SOURCE_DIR}/lib/FreeRTOS/tasks.c"
    "${CMAKE_SOURCE_DIR}/lib/FreeRTOS/timers.c"
    "${CMAKE_SOURCE_DIR}/configuration/entropy_hardware_poll.c"
    "${CMAKE_SOURCE_DIR}/configuration/flash_l4.c"
    "${CMAKE_SOURCE_DIR}/configuration/vl53l0x_platform.c"
    "${CMAKE_SOURCE_DIR}/configuration/vl53l0x_proximity.c"
    "${CMAKE_SOURCE_DIR}/configuration/aws_iot_network_manager.c"
    "${CMAKE_SOURCE_DIR}/configuration/aws_dev_mode_key_provisioning.c"
    "${CMAKE_SOURCE_DIR}/src/peripheral-drivers/i2c.c"
    "${CMAKE_SOURCE_DIR}/src/peripheral-drivers/spi.c"
    "${CMAKE_SOURCE_DIR}/src/peripheral-drivers/uart.c"
    "${CMAKE_SOURCE_DIR}/src/peripheral-drivers/watchdog.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/awsNetworkHandler.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/CLI.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/connectivity.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/flashHandler.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/gpsManager.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/logger.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/otaUpdate.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/pwrMgr.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/taskMonitor.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/memMapHandler.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/mqttHandler.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/ntpHandler.c"
    "${CMAKE_SOURCE_DIR}/src/handlers/updateSsmFw.c"
    "${CMAKE_SOURCE_DIR}/../shared/asp/am-ssm-spi-protocol.c"
    "${CMAKE_SOURCE_DIR}/../shared/asp/am-spi-protocol.c"
    "${CMAKE_SOURCE_DIR}/src/device-drivers/ATECC608A.c"
    "${CMAKE_SOURCE_DIR}/src/device-drivers/externalWatchdog.c"
    "${CMAKE_SOURCE_DIR}/src/device-drivers/mspBslProtocol.c"
    "${CMAKE_SOURCE_DIR}/src/device-drivers/MT29F1.c"
    "${CMAKE_SOURCE_DIR}/src/device-drivers/PE42424A_RF.c"
    "${CMAKE_SOURCE_DIR}/src/device-drivers/sara_u201.c"
    "${CMAKE_SOURCE_DIR}/src/device-drivers/ssm.c"
    "${CMAKE_SOURCE_DIR}/src/application/nwStackFunctionality.c"
    "${CMAKE_SOURCE_DIR}/src/application/eventManager.c"
    "${CMAKE_SOURCE_DIR}/src/main.c"
    "${CMAKE_SOURCE_DIR}/src/stm32l4xx_hal_msp.c"
    "${CMAKE_SOURCE_DIR}/src/stm32l4xx_hal_timebase_TIM.c"
    "${CMAKE_SOURCE_DIR}/src/stm32l4xx_it.c"
    "${CMAKE_SOURCE_DIR}/src/syscalls.c"
    "${CMAKE_SOURCE_DIR}/src/system_stm32l4xx.c"
    "${CMAKE_SOURCE_DIR}/drivers/CMSIS/Device/ST/STM32L4xx/cmsis_os.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rng.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c"
    "${CMAKE_SOURCE_DIR}/drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_wwdg.c"
)
SET_TARGET_PROPERTIES(am PROPERTIES OUTPUT_NAME "am.elf")

# LIBRARIES
TARGET_LINK_LIBRARIES(am -Wl,--start-group)
TARGET_LINK_LIBRARIES(am m)
TARGET_LINK_LIBRARIES(am c)
TARGET_LINK_LIBRARIES(am gcc)
TARGET_LINK_LIBRARIES(am nosys)
TARGET_LINK_LIBRARIES(am -Wl,--end-group)

# MAP FILE
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Xlinker -Map=output.map")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Xlinker -Map=output.map")

# Create hex/bin
ADD_CUSTOM_COMMAND(TARGET am POST_BUILD COMMAND ${CMAKE_OBJCOPY} -Oihex ${ProjDirPath}/am.elf ${ProjDirPath}/am.hex)
ADD_CUSTOM_COMMAND(TARGET am POST_BUILD COMMAND ${CMAKE_OBJCOPY} -Obinary ${ProjDirPath}/am.elf ${ProjDirPath}/am.bin)
