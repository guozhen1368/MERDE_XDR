#include "my_poll.h"

#include "my_loop.h"

#include <aplog.h>

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

const int EPoller::MY_NoneEvent = 0;
const int EPoller::MY_READABLE = EPOLLIN | EPOLLPRI;
const int EPoller::MY_WRITABLE = EPOLLOUT;

namespace
{
	const int kNew = -1;
	const int kAdded = 1;
	const int kDeleted = 2;
}

EPoller::EPoller()
	: epollfd_(::epoll_create(16)),
	  events_(kInitEventListSize)
{
	if (epollfd_ < 0)
	{
		LOGEMERGENCY("EPoller : epollfd_ create failed!");
	}
};

EPoller::~EPoller()
{
	::close(epollfd_);
}

int EPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
	int numEvents = ::epoll_wait(epollfd_,
		&*events_.begin(),
		static_cast<int>(events_.size()),
		timeoutMs);

	if (numEvents > 0)
	{
		fillActiveChannels(numEvents, activeChannels);
		if (numEvents == static_cast<int>(events_.size()))
		{
			events_.resize(events_.size()*2);
		}
	}
	else if (numEvents == 0)
	{
		LOGINFO("EPoller::poll() nothing happended");
	}
	else
	{
		LOGALERT("EPoller::poll()");
	}

	return 0;
}

void EPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
	assert(numEvents <= static_cast<int>(events_.size()));
	for (int i = 0; i < numEvents; ++i)
	{
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);

		//int fd = channel->fd;
		/*ChannelMap::const_iterator it = channels_.find(fd);
		if (it != channels_.end() && it->second == channel)
		{*/
			channel->revents = events_[i].events;
			activeChannels->push_back(channel);
		/*}*/
	}
}

void EPoller::updataEvent(Channel* channel)
{
	const int index = channel->index;
	if (index == kNew || index == kDeleted)
	{
	//	// a new one, add with EPOLL_CTL_ADD
	//	int fd = channel->fd;
	//	if (index == kNew)
	//	{
	//		assert(channels_.find(fd) == channels_.end());
	//		channels_[fd] = channel;
	//	}
	//	else // index == kDeleted
	//	{
	//		assert(channels_.find(fd) != channels_.end());
	//		assert(channels_[fd] == channel);
	//	}
		channel->index = kAdded;
		update(EPOLL_CTL_ADD, channel);
	}
	else
	{
		// update existing one with EPOLL_CTL_MOD/DEL
		/*int fd = channel->fd;
		(void)fd;
		assert(channels_.find(fd) != channels_.end());
		assert(channels_[fd] == channel);
		assert(index == kAdded);*/
		if (channel->events == MY_NoneEvent)
		{
			update(EPOLL_CTL_DEL, channel);
			channel->index = kDeleted;
		}
		else
		{
			update(EPOLL_CTL_MOD, channel);
		}
	}
}

void EPoller::removeEvent(Channel* channel)
{
	//int fd = channel->fd;
	int index = channel->index;
	/*assert(index == kAdded || index == kDeleted);
	size_t n = channels_.erase(fd);
	(void)n;
	assert(n == 1);*/

	if (index == kAdded)
	{
		update(EPOLL_CTL_DEL, channel);
	}
	channel->index = kNew;
}

void EPoller::update(int operation, Channel* channel)
{
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = channel->events;
	event.data.ptr = channel;
	int fd = channel->fd;
	int ep = epollfd_;
	if (::epoll_ctl(ep, operation, fd, &event) < 0)
	{
		if (operation == EPOLL_CTL_DEL)
		{
			LOGERROR("epoll_ctl op del");
		}
		else
		{
			LOGERROR("epoll_ctl op other");
		}
	}
}
