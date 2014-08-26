#include "mg_userdata.h"
#include <aplog.h>

#include <string.h>
#include <stdlib.h>

using std::map;

DequeData::DequeData(const uint64_t &timstamp, void *data)
	:_begin(NULL),
	_end(NULL),
	_procEnd(NULL),
	_curNode(NULL),
	_mmeNode(NULL),
	_xdrProcdu(0),
	_timer(NULL)
{
	this->insert(timstamp, data);
}

DequeData::~DequeData()
{
	del_preNode(_end);
}

void DequeData::reset()
{
	setTimer(NULL);
	del_preNode(getProcEnd());
	setProc(0);
	setMmeNode(NULL);
}

double_link_node_t* DequeData::create_node(const uint64_t &timstamp, void *data)
{
	double_link_node_t *node = NULL;
	node = new double_link_node_t;
	if (NULL == node)
	{
		LOGERROR("failed to new double_link_node_t for DequeData");
		return NULL;
	}

	memset(node, 0, sizeof(double_link_node_t));
	node->xdr_data.timstamp = timstamp;
	node->xdr_data.data = data;

	return node;
}

double_link_node_t *DequeData::find_node(const uint64_t &timstamp)
{
	double_link_node_t *node = _end;
	while (NULL != node)
	{
		//sort basis time
		if (timstamp >= node->xdr_data.timstamp)
		{
			return node;
		}
		node = node->prev;
	}

	return node;
}

void DequeData::insert(const uint64_t &timstamp, void *data)
{
	double_link_node_t* node;
	double_link_node_t* pIndex;

	node = create_node(timstamp, data);
	if (NULL == node)
		return;
	_curNode = node;

	if (NULL == _begin)
	{
		_begin = _end = node;
		node->prev = node->next = NULL;
		
		return;
	}

	pIndex = find_node(timstamp);
	if (NULL == pIndex)
	{
		node->prev = NULL;
		node->next = _begin;
		_begin->prev = node;
		_begin = node;

	}
	else
	{
		node->prev = pIndex;
		node->next = pIndex->next;
		pIndex->next = node;
		if (NULL != node->next)
		{
			node->next->prev = node;
		}
		else
		{
			_end = node;
		}

	}

	return;
}

void DequeData::del_preNode(double_link_node_t *node)
{
	double_link_node_t *tmp = NULL;
	if (NULL != node)
	{
		if (NULL != node->next)
		{
			node->next->prev = NULL;
			_begin = node->next;
		} 
		else
		{
			_begin = _end = NULL;
		}	
	}

	while (NULL != node)
	{
		tmp = node->prev;
		free(node->xdr_data.data);
		delete node;

		node = tmp;
	}
}



/*------------------------------------------------------------------------------------*/


UserData::UserData()
{

}

UserData::~UserData()
{
	DequeData *de = NULL;

	MAP_IMSI::iterator it = _mapImsi.begin();
	while (it != _mapImsi.end())
	{
		de = it->second;
		if (NULL != de)
		{
			delete de;
			de = NULL;
		}

		_mapImsi.erase(it++);
	}
}

DequeData *UserData::insert(const IMSI &imsi, const uint64_t &timstamp, void *data)
{
	DequeData *de = NULL;

	MAP_IMSI::iterator it = _mapImsi.find(imsi);
	if (it == _mapImsi.end())
	{
		de = new DequeData(timstamp, data);
		if (NULL == de)
		{
			LOGERROR("failed to new DequeData for userdata");
			return NULL;
		}
		_mapImsi.insert(MAP_IMSI::value_type(imsi, de));
	}
	else
	{
		de = it->second;
		de->insert(timstamp, data);
	}
	

	return de;
}

void UserData::delImsi(const IMSI &imsi)
{
	DequeData *de = NULL;

	MAP_IMSI::iterator it = _mapImsi.find(imsi);
	if (it != _mapImsi.end())
	{
		de = it->second;
		if (NULL != de)
		{
			delete de;
			de = NULL;
		}

		_mapImsi.erase(it);
	}
}
