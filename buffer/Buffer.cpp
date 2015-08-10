#include "Buffer.h"
#include <stdio.h>

namespace BaseLib
{

const size_t Buffer::kCheapPrepend = 8;
const size_t Buffer::kInitialSize = 1024;
const char Buffer::kCRLF[] = "\r\n";
const char Buffer::kCRLFCRLF[] = "\r\n\r\n";

Buffer::Buffer()
    :   readerIndex_(kCheapPrepend),
        writerIndex_(kCheapPrepend)
{
    buffer_.assign(kInitialSize+kCheapPrepend,0);
    assert(readableBytes() == 0);
    assert(writableBytes() == kInitialSize);
    assert(prependableBytes() == kCheapPrepend);
}

Buffer::Buffer(const Buffer& copy)
{
    buffer_ = copy.buffer_;
    readerIndex_ = copy.readerIndex_;
    writerIndex_ = copy.writerIndex_;
}

Buffer& Buffer::operator =(const Buffer &copy)
{
    buffer_ = copy.buffer_;
    readerIndex_ = copy.readerIndex_;
    writerIndex_ = copy.writerIndex_;
    return *this;
}

void Buffer::swap(Buffer& rhs)
{
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
}

const char* Buffer::findCRLF() const
{
    const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF+2);
    return crlf == beginWrite() ? NULL : crlf;
}

const char* Buffer::findCRLF(const char* start) const
{
    assert(peek() <= start);
    assert(start <= beginWrite());
    const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF+2);
    return crlf == beginWrite() ? NULL : crlf;
}
const char* Buffer::findEOL() const
{
    const void* eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char*>(eol);
}

const char* Buffer::findEOL(const char* start) const
{
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void* eol = memchr(start, '\n', readableBytes());
    return static_cast<const char*>(eol);
}
void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    if (len < readableBytes())
    {
        readerIndex_ += len;
    }
    else
    {
        retrieveAll();
    }
}

void Buffer::retrieveAll()
{
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
}

void Buffer::append(const char* data, size_t len)
{
    ensureWritableBytes(len);
    std::copy(data, data+len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const void* data, size_t len)
{
    append(static_cast<const char*>(data), len);
}

void Buffer::append(const string& data)
{
    append(data.data(),data.length());
}

void Buffer::append(const Buffer& data)
{
    append(data.peek(),data.readableBytes());
}

bool Buffer::appendFromFile(const string path,const string mode)
{
    FILE* pfile = fopen(path.c_str(),mode.c_str());
    int32_t nlength = 0;
    if(pfile)
    {
        fseek(pfile,0,SEEK_END);
        nlength = ftell(pfile);
        if( nlength <= 0)
        {
            fclose(pfile);
            return false;
        }
        fseek(pfile,0,SEEK_SET);
        ensureWritableBytes(nlength);
        fread(beginWrite(),sizeof (int8_t),nlength,pfile);
        hasWritten(nlength);
        fclose(pfile);
		return true;
    }
    return false;
}

size_t Buffer::dumpToFile(const string path, const string mode)
{
    FILE* pfile = fopen(path.c_str(),mode.c_str());
    size_t nlength = 0;
    if(pfile)
    {
        nlength = fwrite(peek(),sizeof (int8_t),readableBytes(),pfile);
        retrieveAll();
        fclose(pfile);
    }
    return nlength;
}

string Buffer::readAllAsString()
{
    return readAsString(readableBytes());
}

string Buffer::readAsString(size_t len)
{
    if(len > readableBytes())
    {
        len = readableBytes();
    }
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

const char* Buffer::readUntilCRLF(string& outLine)
{
    outLine.clear();
    const char* crlf = findCRLF();
    if(crlf)
    {
        assert( crlf < beginWrite());
        outLine.append(peek(),crlf);
        retrieve(crlf+2-peek());
    }
    return crlf;
}

const char* Buffer::readUntilCRLFCRLF(string& outLine)
{
    outLine.clear();
    const char* crlf2 = std::search(peek(), (const char*)beginWrite(), kCRLFCRLF+0, kCRLFCRLF+4);
    crlf2 = (crlf2 == beginWrite())?NULL:crlf2;
    if(crlf2)
    {
        assert( crlf2 < beginWrite());
        outLine.append(peek(),crlf2);
        retrieve(crlf2+4-peek());
    }
    return crlf2;
}

const char* Buffer::peekUntilCRLF(string& outLine)
{
    outLine.clear();
    const char* crlf = findCRLF();
    if(crlf)
    {
        assert( crlf < beginWrite());
        outLine.append(peek(),crlf);
    }
    return crlf;
}

const char* Buffer::peekUntilCRLFCRLF(string& outLine)
{
    outLine.clear();
    const char* crlf2 = std::search(peek(), (const char*)beginWrite(), kCRLFCRLF+0, kCRLFCRLF+4);
    crlf2 = (crlf2 == beginWrite())?NULL:crlf2;
    if(crlf2)
    {
        assert( crlf2 < beginWrite());
        outLine.append(peek(),crlf2);
    }
    return crlf2;
}

void Buffer::prepend(const void* data, size_t len)
{
    assert(len <= prependableBytes());
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d+len, begin()+readerIndex_);
}

void Buffer::shrink(size_t reserve)
{
    // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
    Buffer other;
    other.ensureWritableBytes(readableBytes()+reserve);
    other.append(readAllAsString());
    swap(other);
}

void Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() < len)
    {
        makeSpace(len);
    }
    assert(writableBytes() >= len);
}

void Buffer::makeSpace(size_t len)
{

    if (writableBytes() + prependableBytes() < len + kCheapPrepend)
    {
        // FIXME: move readable data
        buffer_.resize(writerIndex_+len+kCheapPrepend,0);
    }
    else
    {
        // move readable data to the front, make space inside buffer
        assert(kCheapPrepend < readerIndex_);
        size_t readable = readableBytes();
        std::copy(begin()+readerIndex_,
                  begin()+writerIndex_,
                  begin()+kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;

        assert(readable == readableBytes());
    }
}

}
