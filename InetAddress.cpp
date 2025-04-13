#include "InetAddress.h"
#include <strings.h>
#include <cstring>
InetAddress::InetAddress(uint16_t port, std::string ip)
{
    // bzero(&addr_, sizeof(addr_));
    memset(&addr_, 0, sizeof(addr_));
    // 设置ipv4地址族
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);                  // 端口 主机字节序​（小端/大端）转换为 ​网络字节序（大端）​，确保跨平台兼容性
    addr_.sin_addr.s_addr = inet_addr(ip.c_str()); // 地址  "192.168.1.1"
}

std::string InetAddress::toIp() const
{

    char buf[64] = {0};
    // :: 全局命名空间，避免与 C++ 标准库或用户自定义的同名函数冲突
    // 是 ​将 IPv4 地址从二进制格式转换为可读字符串 的标准方法，属于 ​POSIX 套接字编程 的一部分
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    size_t end = strlen(buf);
    uint16_t port = ntohl(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}
uint16_t InetAddress::toPort() const
{
    return ntohs(addr_.sin_port);
}

// #include <iostream>
// int main()
// {
//     InetAddress addr(8080);
//     std::cout << addr.toIpPort() << std::endl;
//     return 0;
// }
