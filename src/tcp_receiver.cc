#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive(TCPSenderMessage message, Reassembler &reassembler, Writer &inbound_stream) {
    if (message.SYN)SYN = true, ISN = message.seqno;
    if (!SYN)return;
    // stream index = absolute seq - 1
    // 注意SYN可能和payload一起来，计算当前payload的absolute seq即可
    reassembler.insert((message.seqno + message.SYN).unwrap(ISN, inbound_stream.bytes_pushed()) - 1, message.payload,
                       message.FIN, inbound_stream);
    return;
}

TCPReceiverMessage TCPReceiver::send(const Writer &inbound_stream) const {
    // Your code here.
    std::optional<Wrap32> ackno{};
    // 坑人的细节
    /*
        若当前无SYN请求，则返回空
        若SYN连接之前已经建立，则返回下一个期望的payload的seq
        因为capacity限制在65,535以内，故下一个期望abs seq=(push_bytes()+1)
        下一个seq = absseq % (1UL<<31) + ISN
        注意：当收到FIN时，不代表当前会话结束，有可能当前reassembler仍然有字符未读出
        故，最终还需要判断当前连接是否已经结束，结束时，相当于增加一个FIN字位
        即，最终答案还要加上inbound_stream.is_closed()
     */
    if (SYN) { ackno = ISN + 1+inbound_stream.is_closed() + inbound_stream.bytes_pushed() % (1UL << 32); }
    return {
            ackno,
            uint16_t(min(inbound_stream.available_capacity(), 1UL * UINT16_MAX)), // 保证在65535以内
    };
}
