#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream(uint64_t capacity) : capacity_(capacity),q(){}

void Writer::push(string data) {
    for (const auto &c: data) {
        if (available_capacity() == 0)break;
        q.push(c);
        writeSize++;
    }
    return;
}

void Writer::close() {
    isClose = 1;
    return;
}

void Writer::set_error() {
    error = 1;
    return;
}

bool Writer::is_closed() const {
    return isClose;
}

uint64_t Writer::available_capacity() const {
    return capacity_ - (uint64_t) q.size();
}

uint64_t Writer::bytes_pushed() const {
    return writeSize;
}

string_view Reader::peek() const {
    return std::string_view(&q.front(), 1);
}

bool Reader::is_finished() const {  // close and fully poped
    return isClose and q.size()==0;
}

bool Reader::has_error() const {
    return error;
}

void Reader::pop(uint64_t len) {
    // Your code here.
    while(len-- and q.size()){
        q.pop();
        readSize++;
    }
}

uint64_t Reader::bytes_buffered() const {
    // Your code here.
    return q.size();
}

uint64_t Reader::bytes_popped() const {
    // Your code here.
    return readSize;
}
