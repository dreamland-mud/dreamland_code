
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <poll.h>

#include "logstream.h"
#include "socketmanager.h"


using namespace std;

SocketManager::SocketManager()
{
}

SocketManager::~SocketManager()
{
}

void 
SocketManager::put(SocketTask::Pointer task)
{
    slay(task);
    push_back(task);
}

inline static bool 
__slay__(SocketTask::Pointer task1, const SocketTask::Pointer &task2)
{
    return task1.getPointer( ) == task2.getPointer( );
}

void 
SocketManager::slay(SocketTask::Pointer task)
{
    remove_if(bind2nd(ptr_fun(__slay__), task));
}

void 
SocketManager::run(int ms)
{
    list<SocketTask::Pointer> clone(begin(), end());
    struct pollfd fds[clone.size()];
    int j = 0;

    clear();

    for(iterator i=clone.begin();i!=clone.end();i++, j++) {
        fds[j].fd = (*i)->descriptor;
        fds[j].events = (*i)->flags;
    }

    int rc = poll(fds, size(), ms);

    if(rc == 0) {
        return;
    }

    if(rc < 0) {
        LogStream::sendError() << "poll: " << strerror(errno) << endl;
        return;
    }
    
    j = 0;
    for(iterator i=clone.begin();i!=clone.end();i++, j++) {
        if(fds[j].revents & POLLIN)
            (*i)->handleRead();
        
        if(fds[j].revents & POLLOUT)
            (*i)->handleWrite();
    }
}

