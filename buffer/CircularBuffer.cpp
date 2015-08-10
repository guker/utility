#include "CircularBuffer.h"
#include <cstdio> //for fopen

namespace BaseLib
{
void CircularBuffer::append(const char *data,size_t len)
{
    m_circular_buffer_.insert(m_circular_buffer_.end(),data,data+len);
}
void CircularBuffer::append(const std::string &sData)
{
    m_circular_buffer_.insert(m_circular_buffer_.end(),sData.begin(),sData.end());
}
void CircularBuffer::append(const CircularBuffer &data)
{
    m_circular_buffer_.insert(m_circular_buffer_.end(),data.m_circular_buffer_.begin(),
                              data.m_circular_buffer_.end());
}
void CircularBuffer::append(const Buffer& buffer)
{
    m_circular_buffer_.insert(m_circular_buffer_.end(),
                              buffer.peek(),buffer.peek()+buffer.readableBytes());
}

int32_t CircularBuffer::appendFromFile(const std::string path,const std::string mode)
{
    FILE *fp = fopen(path.c_str(),mode.c_str());
    int32_t nLength = -1;
    if (fp)
    {
        fseek(fp,0,SEEK_END);
        nLength = ftell(fp);
        fseek(fp,0,SEEK_SET);
        int8_t *pContent = new int8_t[nLength+1]();
        fread(pContent,1,nLength,fp);
        m_circular_buffer_.assign(&pContent[0],&pContent[nLength]);
        delete[] pContent;
        pContent = NULL;
    }
    fclose(fp);
    return nLength;
}

std::string CircularBuffer::readAllAsString()
{
    std::string sRes("");
    sRes.append(m_circular_buffer_.begin(),m_circular_buffer_.end());
    return sRes;
}
std::string CircularBuffer::readAsString(uint32_t nLen, uint32_t nOffset, bool from_end)
{
    std::string sRes;

    if(nOffset + nLen <= m_circular_buffer_.size() && nOffset >= 0)
    {
        if(from_end)
        {
            sRes.append(m_circular_buffer_.end()-nOffset-nLen,
                        m_circular_buffer_.end()-nOffset);
        }
        else
        {
            sRes.append(m_circular_buffer_.begin()+nOffset,
                        m_circular_buffer_.begin()+nOffset+nLen);
        }
    }
    return sRes;
}

Buffer CircularBuffer::readAllAsBuffer()
{
    Buffer buffer;
    std::string str = readAllAsString();
    if(!str.empty())
    {
        buffer.append(str);
    }
    return buffer;
}

Buffer CircularBuffer::readAsBuffer(uint32_t nLen, uint32_t nOffset, bool from_end)
{
    Buffer buffer;
    std::string str = readAsString(nLen,nOffset,from_end);
    if(!str.empty())
    {
        buffer.append(str);
    }
    return buffer;
}

bool CircularBuffer::readAsBuffer(Buffer &out,uint32_t nLen, uint32_t nOffset, bool from_end)
{
	out.retrieveAll();
	std::string str = readAsString(nLen,nOffset,from_end);
	if(!str.empty())
	{
		out.append(str);
	}
	return true;
}

CircularBuffer::CircularBuffer(CircularBuffer &rhs)
{
    m_circular_buffer_ = rhs.m_circular_buffer_;
}

CircularBuffer& CircularBuffer::operator = (const CircularBuffer &rhs)
{
    m_circular_buffer_ = rhs.m_circular_buffer_;
    return *this;
}
}

