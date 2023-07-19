#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender(uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn)
        : isn_(fixed_isn.value_or(Wrap32{random_device()()})),
          initial_RTO_ms_(initial_RTO_ms), timer(initial_RTO_ms) {}

uint64_t TCPSender::sequence_numbers_in_flight() const {
    return next_seq-recv_seq;
}

uint64_t TCPSender::consecutive_retransmissions() const {
    // Your code here.
    return retransmissions_num;
}

optional<TCPSenderMessage> TCPSender::maybe_send() {
//    cout<<windowSize<<"-------"<<endl;
    if (!SYN)return {};
    else if (sending_queue.size()) { // 如果当前发送队列有包的话
        if (!timer.getStartStatus()) { // 若当前未开始，开始计时
            timer.updateStartStatus(true); // 设置status
            timer.reset();                        // 重置RTO
        }
        auto msg = sending_queue.front();
        outstanding_queue.push(msg);
        sending_queue.pop();
        return msg;
    }
    return {};
}

void TCPSender::push(Reader &outbound_stream) {
//    cout<<windowSize<<endl;
    if (FIN)return;
    uint64_t remainWindowSize = max(windowSize, uint16_t(1)) - (next_seq - recv_seq); // 当前还能发送的数据的大小（不包括SYN和FIN）
    while (remainWindowSize and !FIN) {
        TCPSenderMessage msg;
        Buffer &buffer = msg.payload;
        read(outbound_stream, uint64_t(min(size_t(remainWindowSize), TCPConfig::MAX_PAYLOAD_SIZE)), buffer);
        remainWindowSize -= msg.payload.size();
        msg.seqno = Wrap32::wrap(next_seq, isn_);
        if (!SYN) {
            msg.SYN = true;
            SYN = true;
        }
        if (!FIN and outbound_stream.is_finished() and remainWindowSize) {
            FIN = true;
            msg.FIN = true;
        }
        if (!msg.SYN and !msg.FIN and !msg.payload.size()) {
//            cout<<windowSize<<' '<<msg.payload.size()<<endl;
            return; // 若当前不发送SYN或者FIN，且无任何数据则返回
        }
        sending_queue.push(msg);
        next_seq += msg.sequence_length();
    }
}

TCPSenderMessage TCPSender::send_empty_message() const {
    TCPSenderMessage msg;
    msg.payload = {};
    msg.seqno = Wrap32::wrap(next_seq, isn_);
    return msg;
}

void TCPSender::receive(const TCPReceiverMessage &msg) {
    retransmissions_num = 0;
    if (msg.ackno.has_value() == 0)return;
    uint64_t tcp_recv_seq = msg.ackno->unwrap(isn_, recv_seq);
    if (tcp_recv_seq > next_seq or tcp_recv_seq<recv_seq)return; // 不合法
    windowSize = msg.window_size;
    while (outstanding_queue.size()) {
        auto &message = outstanding_queue.front();
//        cout<<message.sequence_length()<<' '<<(message.seqno.unwrap(isn_, recv_seq))<<' '<<recv_seq<<endl;
        if ((message.seqno.unwrap(isn_, recv_seq) + message.sequence_length() - 1) < tcp_recv_seq) {
            outstanding_queue.pop();
        } else break;
    }
    if(outstanding_queue.size()){
        if(tcp_recv_seq>recv_seq)timer.reset();
    }else timer.updateStartStatus(false);
    recv_seq = tcp_recv_seq;
}

void TCPSender::tick(const size_t ms_since_last_tick) {
    timer.tick(ms_since_last_tick); // 时间流逝
//    cout << timer.getRTO() << endl;
    if (timer.getStartStatus() and timer.isExpire()) { // 若当前timer开始，且已经过期了
        while (outstanding_queue.size()) {
            auto &message = outstanding_queue.front();
            if ((message.seqno.unwrap(isn_, recv_seq) + message.sequence_length() - 1) < recv_seq) {
                outstanding_queue.pop();
            } else break;
        }
        if (outstanding_queue.size()) { // 发送的包超时了，重新发送
            auto msg = outstanding_queue.front();
            sending_queue.push(msg);
            retransmissions_num++;
            // PDF: If the window size is nonzero: Double the value of RTO
            // 隐藏情况：当SYN未成功连接时，window size始终为0，对此情况，用recv_seq进行特判
            if(!recv_seq || windowSize)timer.doubleRTO();
            else timer.reset();   // 当window size为0时，此时发送的是试探报文，故timer的RTO需要重置成初始值
        }else timer.updateStartStatus(false); // 当前无发送的包了，timer可以关闭
    }
}
