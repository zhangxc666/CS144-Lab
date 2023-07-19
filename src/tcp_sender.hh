#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

class Timer {
    int64_t initialTime; // 初始化时间
    int64_t fixTime;     // 固定的时间，RTO=fixtime
    int64_t RTO{0};      // RTO 倒计时，会随时间变化
    bool isStart{false};  // 是否开始
public:
    Timer(size_t t) : initialTime(t),fixTime(t){}

    void reset() {
        RTO = initialTime;
        fixTime=initialTime;
        isStart = true;
    } // 重置RTO

    int64_t getRTO() { return RTO; }  // 获取RTO

    bool isExpire() { return RTO <= 0; } // 是否过期

    void tick(uint64_t ms) { RTO -= ms; } // 通过调用tick模拟时间流逝

    bool getStartStatus() { return isStart; } // 获取开始状态

    void updateStartStatus(bool status) { isStart = status; } // 更新开始状态

    void doubleRTO() { fixTime*=2;RTO=fixTime;isStart= true; } // 翻倍 RTO
};

class TCPSender {
    Wrap32 isn_;             // isn
    uint64_t initial_RTO_ms_;
    Timer timer;            // 计时器
    uint64_t retransmissions_num{0}; // 重新传输的包的数量
    uint64_t recv_seq{0};  // 当前已经发送过且成功接受的seqno  <==> 已发送窗口的left
    uint64_t next_seq{0};  // 下一个要发送的seqno             <==> =已发送窗口的right+1
    uint16_t windowSize{0};        // receiver的窗口大小
    bool SYN{false};       // 当前是否发送过SYN;
    bool FIN{false};       // 是否发送过FIN
    std::queue<TCPSenderMessage> outstanding_queue {}; // 已发送还未接受的queue
    std::queue<TCPSenderMessage> sending_queue   {}; // 还未发送的queue
public:
    /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
    TCPSender(uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn);

    /* Push bytes from the outbound stream */
    void push(Reader &outbound_stream);

    /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
    std::optional<TCPSenderMessage> maybe_send();

    /* Generate an empty TCPSenderMessage */
    TCPSenderMessage send_empty_message() const;

    /* Receive an act on a TCPReceiverMessage from the peer's receiver */
    void receive(const TCPReceiverMessage &msg);

    /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
    void tick(uint64_t ms_since_last_tick);

    /* Accessors for use in testing */
    uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
    uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
