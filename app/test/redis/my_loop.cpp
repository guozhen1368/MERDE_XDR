#include "my_loop.h"

#include <string.h>
#include <stdio.h>


const int kPollTimeMs = 10000;

EventLoop::EventLoop()
	: looping_(false),
	  quit_(false),
	  currentActiveChannel_(NULL)
{

}

EventLoop::~EventLoop()
{

}

void EventLoop::loop()
{
	looping_ = true;
	quit_ = false;

	while (!quit_)
	{
		activeChannels_.clear();
		poller_.poll(kPollTimeMs, &activeChannels_);

		// TODO sort channel by priority
		for (ChannelList::iterator it = activeChannels_.begin();
			it != activeChannels_.end(); ++it)
		{
			Channel * l = *it;
			if (l->events & EPoller::MY_READABLE)
			{
				l->readCallback(this, l->fd, l->privdata, 0);
			}

			if (l->events & EPoller::MY_WRITABLE)
			{
				l->writeCallback(this, l->fd, l->privdata, 0);
			}
			
			currentActiveChannel_ = l;
		}
		currentActiveChannel_ = NULL;

		//doPendingFunctors();
	}

	looping_ = false;
}

void EventLoop::quit()
{
	quit_ = true;
}

void EventLoop::createFileEvent(int fd, int events, EventCallBack *callback, void *privdata)
{
	Channel * chan = NULL;
	Channelmap::iterator it = channels_.find(fd);
	if ( it == channels_.end())
	{
		chan = new Channel;
		if ( NULL == chan)
		{
			printf("EventLoop : createFileEvent new Channel is null");
			return;
		}
		memset(chan, 0, sizeof(Channel));

		chan->fd = fd;
		if (events == EPoller::MY_READABLE)
		{
			chan->readCallback = callback;
		}

		if (events == EPoller::MY_WRITABLE)
		{
			chan->writeCallback = callback;
		}

		chan->events = events;
		chan->index = -1;
		chan->privdata = privdata;
		channels_[fd] = chan;
	}
	else
	{
		chan = it->second;

		if (events == EPoller::MY_READABLE)
		{
			chan->readCallback = callback;
		}

		if (events == EPoller::MY_WRITABLE)
		{
			chan->writeCallback = callback;
		}

		chan->events |= events;
	}
	
	poller_.updataEvent(chan);
}

void EventLoop::deleteFileEvent(int fd, int events)
{
	Channelmap::iterator it = channels_.find(fd);
	if (it!=channels_.end())
	{
		Channel * chan = it->second;
		chan->events = events;
		poller_.removeEvent(chan);
	}
}
