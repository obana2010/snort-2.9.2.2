/****************************************************************************
 *
 * Copyright (C) 2004-2012 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/
 
#ifndef __SF_EVENTQ_H__
#define __SF_EVENTQ_H__

#define NUM_EVENT_QUEUES 3

typedef struct s_SF_EVENTQ_NODE
{
    void   *event;

    struct s_SF_EVENTQ_NODE *prev;
    struct s_SF_EVENTQ_NODE *next;

}  SF_EVENTQ_NODE;

typedef struct s_SF_EVENTQ
{
    /*
    **  Handles the actual ordering and memory
    **  of the event queue and it's nodes.
    */
    SF_EVENTQ_NODE *head;
    SF_EVENTQ_NODE *last;

    SF_EVENTQ_NODE *node_mem;
    char           *event_mem;

    /*
    **  The reserve event allows us to allocate one extra node
    **  and compare against the last event in the queue to determine
    **  if the incoming event is a higher priority than the last 
    **  event in the queue.
    */
    char           *reserve_event;
    
    /*
    **  Queue configuration
    */
    int max_nodes;
    int log_nodes;
    int event_size;

    /*
    **  This function orders the events as they
    **  arrive.
    */
    int (*sort)(void *event1, void *event2);

    /*
    **  This element tracks the current number of
    **  nodes in the event queue.
    */
    int cur_nodes;
    int cur_events;

}  SF_EVENTQ;


SF_EVENTQ * sfeventq_new(int max_nodes, int log_nodes, int event_size, 
                         int (*sort)(void *, void *));

void * sfeventq_event_alloc(SF_EVENTQ *);
void sfeventq_reset(SF_EVENTQ *);
int sfeventq_add(SF_EVENTQ *, void *event);
int sfeventq_action(SF_EVENTQ *, int (*action_func)(void *event, void *user), void *user);
void sfeventq_free(SF_EVENTQ *);

#endif
