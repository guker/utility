#ifndef __CIRCULARBUFFERF_H__
#define __CIRCULARBUFFERF_H__

#include <string>
#include <boost/circular_buffer.hpp>
#include "buffer/Buffer.h"

#define  CIRCUALR_BUFFER_SIZE 1024*1024*1 //2MB

namespace BaseLib
{
	class CircularBuffer
	{
	public:
		void append(const char *data,size_t len);
		void append(const std::string &sData);
		void append(const CircularBuffer &data);
		void append(const Buffer &buffer);

		int32_t appendFromFile(const std::string path,const std::string mode = "rb");
		std::string readAllAsString();
		std::string readAsString(uint32_t nLen,uint32_t nOffset = 0,bool from_end = true);

		Buffer readAllAsBuffer();
        Buffer readAsBuffer(uint32_t nLen, uint32_t nOffset, bool from_end);
		bool readAsBuffer(Buffer &out,uint32_t nLen, uint32_t nOffset, bool from_end);

		CircularBuffer(CircularBuffer &rhs);
		CircularBuffer& operator = (const CircularBuffer &rhs);

	public:
		CircularBuffer(uint32_t nSize = CIRCUALR_BUFFER_SIZE)
			:m_circular_buffer_(nSize)
		  {
		  }
		~CircularBuffer(void)
		{

		}
		size_t size()
		{
			return m_circular_buffer_.size();
		}

		void clear()
		{
			m_circular_buffer_.clear();
		}
	private:
		boost::circular_buffer<char> m_circular_buffer_;
	};

}

#endif // __CIRCULARBUFFERF_H__
