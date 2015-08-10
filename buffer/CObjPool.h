#ifndef COBJPOOL_H
#define COBJPOOL_H

#include "boost/pool/object_pool.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/thread/lock_guard.hpp"
#include "boost/bind.hpp"

namespace BaseLib
{

template<class T>
class CObjPool : boost::noncopyable
{
public:

    CObjPool(){}

    virtual ~CObjPool(){}

    boost::shared_ptr<T> CreateObject()
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        T* p = pool_.construct();
        assert(p);
        boost::shared_ptr<T> ptr(p,boost::bind(&CObjPool::DestroyObject,this,_1) );
        return ptr;
    }

    template <class Arg1>
    boost::shared_ptr<T> CreateObject(Arg1 a1)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        T* p = pool_.construct(a1);
        assert(p);
		boost::shared_ptr<T> ptr(p,boost::bind(&CObjPool::DestroyObject,this,_1) );
        return ptr;
    }

    template <class Arg1, class Arg2>
    boost::shared_ptr<T> CreateObject(Arg1 a1,Arg2 a2)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        T* p = pool_.construct(a1,a2);
        assert(p);
        boost::shared_ptr<T> ptr(p,boost::bind(&CObjPool::DestroyObject,this,_1) );
        return ptr;
    }

protected:

    void DestroyObject(T* ptr)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        if(ptr != NULL && pool_.is_from(ptr))
        {
            pool_.destroy(ptr);
        }
    }

private:
    boost::object_pool<T> pool_;
    boost::mutex mutex_;
};
}

#endif // COBJPOOL_H
