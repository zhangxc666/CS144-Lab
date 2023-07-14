#pragma once

#include <queue>
#include <stdexcept>
#include <string>
#include <string_view>

class Reader;

class Writer;

class ByteStream {
protected:
    uint64_t capacity_;  // 容量
    std::queue<char> q;  // 存储值
    uint64_t writeSize = 0;  // 写的数量
    uint64_t readSize = 0;   // 读的数量
    bool isClose = false;   // 是否结束
    bool error = 0;     // 是否错误

public:
    explicit ByteStream(uint64_t capacity);

    Reader &reader();

    const Reader &reader() const;

    Writer &writer();

    const Writer &writer() const;
};

class Writer : public ByteStream {
public:
    void push(std::string data); // Push data to stream, but only as much as available capacity allows.

    void close();     // Signal that the stream has reached its ending. Nothing more will be written.
    void set_error(); // Signal that the stream suffered an error.

    bool is_closed() const;              // Has the stream been closed?
    uint64_t available_capacity() const; // How many bytes can be pushed to the stream right now?
    uint64_t bytes_pushed() const;       // Total number of bytes cumulatively pushed to the stream
};

class Reader : public ByteStream {
public:
    std::string_view peek() const; // Peek at the next bytes in the buffer
    void pop(uint64_t len);      // Remove `len` bytes from the buffer

    bool is_finished() const; // Is the stream finished (closed and fully popped)?
    bool has_error() const;   // Has the stream had an error?

    uint64_t bytes_buffered() const; // Number of bytes currently buffered (pushed and not popped)
    uint64_t bytes_popped() const;   // Total number of bytes cumulatively popped from stream
};

/*
 * read: A (provided) helper function thats peeks and pops up to `len` bytes
 * from a ByteStream Reader into a string;
 */
void read(Reader &reader, uint64_t len, std::string &out);
