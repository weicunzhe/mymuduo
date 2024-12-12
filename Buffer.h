#pragma once

#include <vector>
#include <string>
#include <algorithm>

// 网络库底层的缓冲器类定义
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend) {};

    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writebleBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const { return readerIndex_; }

    // 返回缓冲区中可读数据的起始地址
    const char *peek() const { return begin() + readerIndex_; }

    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            // 应用只读取了可读缓冲区数据一部分，就是len, 还剩下 readerIndex_ += len ->writerIndex_
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    // 把 onMessage函数上报的Buffer数据，转成string类型的数据返回
    std::string retrieveAllString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(), len);
        retrieve(len); // 上面一句把缓冲区中可读的数据，已经读取出来，这里肯定是要对缓冲区进行复位操作
        return result;
    }

    // buffer_.size - writeIndex_  len
    void ensureWriteableBytes(size_t len)
    {
        if (writebleBytes() < len)
        {
            makeSpace(len); // 扩容函数
        }
    }

    
    // 把data, data + len内存上的数据, 添加到writable缓冲区当中
    void append(const char *data, size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    char *beginWrite() { return begin() + writerIndex_; }
    const char *beginWrite() const { return begin() + writerIndex_; }

    // 从fd上读取数据
    ssize_t readFd(int fd, int *saveErrno);

private:
    // it.operator*().operator&()
    char *begin() { return &*buffer_.begin(); } // vector底层数组首元素的地址，也就是数组的起始地址
    const char *begin() const { return &*buffer_.begin(); }

    void makeSpace(size_t len)
    {
        if (writebleBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

private:
    std::vector<char>
        buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};