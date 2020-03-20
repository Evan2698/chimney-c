/*******************************************************************************
 *
 * buffer for socket
 * 
 *
 * 
 *
 ******************************************************************************/

#ifndef BUFFER_H_FUCJ_3842948
#define BUFFER_H_FUCJ_3842948
#include <mutex>
#include <vector>
#include <queue>
#include <memory>

struct Buffer
{ 
    typedef  std::vector<unsigned char> BUFFERTYPE;

    Buffer()=default;
    ~Buffer();
    std::shared_ptr<BUFFERTYPE> pop();
    void push(std::shared_ptr<BUFFERTYPE> e);
    bool empty() const;

private:
    std::mutex _mutex;   
    std::queue<std::shared_ptr<BUFFERTYPE> > queue;    
};

#endif