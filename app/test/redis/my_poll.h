#ifndef __MY_POLL_H__
#define __MY_POLL_H__

#include <boost/noncopyable.hpp>

#include <map>
#include <vector>

struct epoll_event;

struct Channel;

class EPoller : boost::noncopyable
{
public:
	typedef std::vector<Channel*> ChannelList;

	EPoller();
	~EPoller();

	int poll(int timeoutMs, ChannelList* activeChannels);
	void updataEvent(Channel* channel);
	void removeEvent(Channel* channel);

	static const int MY_NoneEvent;
	static const int MY_READABLE;
	static const int MY_WRITABLE;

private:
	static const int kInitEventListSize = 16;

	void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
	void update(int operation, Channel* channel);

	typedef std::vector<struct epoll_event> EventList;
	typedef std::map<int, Channel*> ChannelMap;

	int epollfd_;
	EventList events_;
	//ChannelMap channels_;

};

#endif  //__MY_POLL_H__