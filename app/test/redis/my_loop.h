#ifndef __MY_LOOP_H__
#define __MY_LOOP_H__

#include <map>

#include "my_poll.h"

typedef void EventCallBack(class EventLoop *el, int fd, void *privdata, int mask);
typedef struct Channel{
	int  fd;
	int  events;
	int  revents;
	int  index;
	EventCallBack *readCallback;
	EventCallBack *writeCallback;
	void *privdata;
}Channel;

class EventLoop : boost::noncopyable
{
public:
	EventLoop();
	~EventLoop();

	void loop();
	void quit();

	void createFileEvent(int fd, int events, EventCallBack *callback, void *privdata);
	void deleteFileEvent(int fd, int events);


private:
	typedef std::vector<Channel*> ChannelList;
	typedef std::map<int, Channel*> Channelmap;

	bool looping_; /* atomic */
	bool quit_; /* atomic */

	ChannelList activeChannels_;
	EPoller poller_;
	Channel* currentActiveChannel_;
	Channelmap channels_;
	
};

#endif  //__MY_LOOP_H__