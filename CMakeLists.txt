cmake_minimum_required(VERSION VERSION 3.10)
project(httpproxy_dev C)

set(CMAKE_C_STANDARD 99)

add_executable(httpproxy_dev main.c logger.h
        logger.c netutils.c netutils.h callback.c callback.h net.c net.h
        http_message.c http_message.h proxy.h
        list.h list.c)

TARGET_LINK_LIBRARIES(httpproxy_dev ev)
set(CMAKE_C_FLAGS_DEBUG "-lev")

#curl -x http://username:password@ip:port -I destIp
#curl -x http://127.0.0.1:1090 -I http://www.baidu.com