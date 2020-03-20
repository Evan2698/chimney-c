#include "common/buffer.h"

Buffer::~Buffer()
{
    this->queue.empty();
}

std::shared_ptr<Buffer::BUFFERTYPE> Buffer::pop()
{

    std::shared_ptr<Buffer::BUFFERTYPE> sp;
    {
        std::lock_guard<std::mutex> lock(this->_mutex);
        sp = this->queue.front();
        this->queue.pop();
    }
    return sp;
}

void Buffer::push(std::shared_ptr<Buffer::BUFFERTYPE> e)
{
    std::lock_guard<std::mutex> lock(this->_mutex);
    this->queue.push(e);
}

bool Buffer::empty() const{
   return  this->queue.empty();
}