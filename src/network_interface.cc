#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
        : ethernet_address_(ethernet_address), ip_address_(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(ethernet_address_) << " and IP address "
         << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    EthernetFrame frame;
    uint32_t ip = next_hop.ipv4_numeric();
    Serializer ser;
    // 若当前IP对应的MAC可以查得到 且 存储数据的时间未超时
    if (IptoMac.count(ip) and curTime - IptoMac[ip].second <= (1ULL * 30 * 1000)) {
        frame.header.type = EthernetHeader::TYPE_IPv4; // 封装上层数据包类型
        frame.header.src = ethernet_address_;          // 封装原地址
        frame.header.dst = IptoMac[ip].first;    // 封装目的地址
        dgram.serialize(ser);
    } else { // 对应MAC未查到，将IP存起来,五秒内不发同一个ARP请求
        if (lastARPRequestTime.count(ip) and (curTime - lastARPRequestTime[ip]) <= (1ULL * 5 * 1000))return;
        else lastARPRequestTime[ip] = curTime;
        ARPMessage arpmsg;
        frame.header.type = EthernetHeader::TYPE_ARP;    // 设置帧头部
        frame.header.src = ethernet_address_;
        frame.header.dst = ETHERNET_BROADCAST;
        arpmsg.opcode = ARPMessage::OPCODE_REQUEST;        // 设置ARP请求
        arpmsg.sender_ip_address = ip_address_.ipv4_numeric();
        arpmsg.sender_ethernet_address = ethernet_address_;
        arpmsg.target_ip_address = ip;
        arpmsg.target_ethernet_address = EthernetAddress{0, 0, 0, 0, 0, 0};
        arpmsg.serialize(ser);
        IP_queue[ip].push(dgram);             // 将IP数据报存进对应的队列中，待变成frame
    }
    frame.payload = ser.output();                  // 封装负载
    frame_queue.push(frame);  // 推进帧队列中，待发送
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if (frame.header.dst != ETHERNET_BROADCAST and frame.header.dst != ethernet_address_)return {};
    if (frame.header.type == EthernetHeader::TYPE_IPv4) { // 如果上层数据是IPV4
        InternetDatagram networkData;
        if (!parse(networkData, frame.payload))return {}; // 解析失败
        return networkData;
    } else if (frame.header.type == EthernetHeader::TYPE_ARP) { // 如果是ARP
        ARPMessage msg;
        if (!parse(msg, frame.payload))return {};
        IptoMac[msg.sender_ip_address] = {msg.sender_ethernet_address, curTime};
        if (msg.opcode == ARPMessage::OPCODE_REQUEST) { // ARP请求
            if(msg.target_ip_address!=ip_address_.ipv4_numeric())return {}; // 当前目标IP地址错误
            ARPMessage sendARPmsg;
            EthernetFrame sendFrame;
            Serializer ser;
            sendFrame.header.type = EthernetHeader::TYPE_ARP;
            sendFrame.header.dst = msg.sender_ethernet_address;
            sendFrame.header.src = ethernet_address_;
            sendARPmsg.opcode=ARPMessage::OPCODE_REPLY;
            sendARPmsg.sender_ethernet_address=ethernet_address_;
            sendARPmsg.sender_ip_address=ip_address_.ipv4_numeric();
            sendARPmsg.target_ethernet_address=msg.sender_ethernet_address;
            sendARPmsg.target_ip_address=msg.sender_ip_address;
            sendARPmsg.serialize(ser);
            sendFrame.payload = ser.output();
            frame_queue.push(sendFrame);
        }
        if (IP_queue.count(msg.sender_ip_address)) {  // 把未发送的IP数据包重新转换成帧发送
            auto &queue = IP_queue[msg.sender_ip_address];
            while (queue.size()) {
                EthernetFrame sendFrame;
                Serializer ser;
                auto ipdata = queue.front();
                queue.pop();
                sendFrame.header.type = EthernetHeader::TYPE_IPv4;
                sendFrame.header.src = ethernet_address_;          // 封装原地址
                sendFrame.header.dst = msg.sender_ethernet_address;// 封装目的地址
                ipdata.serialize(ser);
                sendFrame.payload = ser.output();
                frame_queue.push(sendFrame);
            }
        }
    }
    return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    curTime += (uint64_t) ms_since_last_tick;
}

optional<EthernetFrame> NetworkInterface::maybe_send() {
    if (frame_queue.size()) {
        auto frame = frame_queue.front();
        frame_queue.pop();
        return frame;
    }
    return {};
}
