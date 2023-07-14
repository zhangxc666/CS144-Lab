#pragma once

#include "byte_stream.hh"

#include <string>
#include <set>
#include <map>
using namespace std;
typedef pair<uint64_t , uint64_t> PII;

class Reassembler {
private:
    bool isFinish;
    uint64_t pushtotalBytes; // 当前已经推进去的字节数
    uint64_t totalBytes;     // 当前存储的字节数
    set<PII> s;// 维护一个区间
    map<uint64_t,string> mp; // 当前下标对应的字符串
    void mergeString(int lmin,int l,int r,string &dest,string &src);
public:
    Reassembler():isFinish(false),pushtotalBytes(0),totalBytes(0),s(),mp(){}
    /*
     * Insert a new substring to be reassembled into a ByteStream.
     *   `first_index`: the index of the first byte of the substring
     *   `data`: the substring itself
     *   `is_last_substring`: this substring represents the end of the stream
     *   `output`: a mutable reference to the Writer
     *
     * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
     * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
     * learns the next byte in the stream, it should write it to the output.
     *
     * If the Reassembler learns about bytes that fit within the stream's available capacity
     * but can't yet be written (because earlier bytes remain unknown), it should store them
     * internally until the gaps are filled in.
     *
     * The Reassembler should discard any bytes that lie beyond the stream's available capacity
     * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
     *
     * The Reassembler should close the stream after writing the last byte.
     */
    void insert(uint64_t first_index, std::string data, bool is_last_substring, Writer &output);

    // How many bytes are stored in the Reassembler itself?
    uint64_t bytes_pending() const;
};
