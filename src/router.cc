#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/"
         << static_cast<int>( prefix_length ) << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)")
         << " on interface " << interface_num << "\n";
    routes.push_back({route_prefix, prefix_length, next_hop, interface_num});
}

void Router::route() {
    for (auto &interface: interfaces_) {
        while (true) {
            auto ipdata = interface.maybe_receive();  // 获取当前interface的ip数据报
            if (ipdata.has_value() == 0)break;         // 当前interface的数据已经处理完了
            auto ipData = *ipdata;
            int maxx = -1, n = routes.size();
            if (ipData.header.ttl == 0)continue;       // TTL为0，丢弃
            ipData.header.ttl--;
            if (ipData.header.ttl == 0)continue;       // TTL为0，丢弃
            ipData.header.compute_checksum();          // 恶心死了，文档也没提，要更新校验和==
            for (int i = 0; i < n; i++) {                   // 选择最大匹配路由
                auto route = routes[i];
                uint8_t len = 32 - route.prefix_length;
                if ((uint64_t(route.route_prefix) >> len) == (uint64_t(ipData.header.dst) >> len)) {
                    if (maxx == -1 || routes[maxx].prefix_length < route.prefix_length)maxx = i;
                }
            }
            if (maxx == -1)continue;                   // 未找到匹配路由，丢弃
            auto nextHop = routes[maxx].next_hop;
            if (nextHop.has_value() == 0)
                this->interface(size_t(routes[maxx].interface_num))
                        .send_datagram(ipData, Address::from_ipv4_numeric(ipData.header.dst));
            else
                this->interface(size_t(routes[maxx].interface_num)).send_datagram(ipData, *nextHop);
        }
    }
}
