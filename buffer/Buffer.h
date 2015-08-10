#ifndef _BUFFER_H
#define _BUFFER_H

#include <algorithm>
#include <vector>
#include <string.h>
#include <assert.h>
#include <string>
#include "stdint.h"
using namespace std;

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

namespace BaseLib
{

class Buffer
{
public:
    static const size_t kCheapPrepend;
    static const size_t kInitialSize;

    Buffer();
    //  deep copy-ctor, dtor and assignment are fine
    Buffer(const Buffer &copy);

    Buffer& operator = (const Buffer &copy);

    void swap(Buffer& rhs);

    const char* findCRLF() const;

    const char* findCRLF(const char* start) const;

    const char* findEOL() const;

    const char* findEOL(const char* start) const;

    ///
    /// retrieve
    ///

    void retrieve(size_t len);

    void retrieveInt32()
    {
        retrieve(sizeof(int32_t));
    }

    void retrieveInt16()
    {
        retrieve(sizeof(int16_t));
    }

    void retrieveInt8()
    {
        retrieve(sizeof(int8_t));
    }

    void retrieveAll();

    ///
    /// get point and size
    ///
    const char* peek() const
    {
        return begin() + readerIndex_;
    }
    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    void hasWritten(size_t len)
    {
        writerIndex_ += len;
    }

    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    ///
    /// Append
    ///
    void appendInt32(int32_t x)
    {
        int32_t be32 = x;
        append(&be32, sizeof be32);
    }

    void appendInt16(int16_t x)
    {
        int16_t be16 = x;
        append(&be16, sizeof be16);
    }

    void appendInt8(int8_t x)
    {
        append(&x, sizeof x);
    }

    size_t dumpToFile(const string path,const string mode);

    bool appendFromFile(const string path,const string mode);

    void append(const char* /*restrict*/ data, size_t len);

    void append(const void* /*restrict*/ data, size_t len);

    void append(const string &data);

    void append(const Buffer &data);

    ///
    /// Read
    ///
    int32_t readInt32()
    {
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }

    int16_t readInt16()
    {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }

    int8_t readInt8()
    {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    string readAllAsString();

    string readAsString(size_t len);

    ///  outline不包含“\r\n”字段
    const char* readUntilCRLF(string &outLine);
    ///  outline不包含“\r\n”字段
    const char* readUntilCRLFCRLF(string &outLine);

    ///
    /// Peek
    ///

    int32_t peekInt32() const
    {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof be32);
        return be32;
    }

    int16_t peekInt16() const
    {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        return be16;
    }

    int8_t peekInt8() const
    {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }
    ///  outline不包含“\r\n”字段
    const char* peekUntilCRLF(string &outLine);
    ///  outline不包含“\r\n”字段
    const char* peekUntilCRLFCRLF(string &outLine);

    ///
    /// Prepend int32_t using network endian
    ///
    void prependInt32(int32_t x)
    {
        int32_t be32 = x;
        prepend(&be32, sizeof be32);
    }

    void prependInt16(int16_t x)
    {
        int16_t be16 = x;
        prepend(&be16, sizeof be16);
    }

    void prependInt8(int8_t x)
    {
        prepend(&x, sizeof x);
    }

    void prepend(const void* /*restrict*/ data, size_t len);

    void shrink(size_t reserve);

    void ensureWritableBytes(size_t len);

private:

    char* begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const
    {
        return &*buffer_.begin();
    }


    void makeSpace(size_t len);

private:

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];
    static const char kCRLFCRLF[];
};

}
#endif  // _BUFFER_H
