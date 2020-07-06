http代理服务器

**http服务器代理过程**

1.解析头部

2.连接目标地址

3.服务端发送请求信息到目标地址

4.服务端获取响应

5.服务端发送响应信息到客户端

6.客户端获取响应

默认请求地址: 127.0.0.1:1090,如有更改端口需要可在main.c文件中更改，无getopt解析选项,此代理服务器为阻塞型服务器,没有用到高级io。

运行方式: `./httpproxy_dev`

测试方法: `curl -x http://127.0.0.1:1090 -I http://example.com`

测试环境: Ubuntu 16.04

logger版本：0.1.0

https://github.com/shiffthq/logger