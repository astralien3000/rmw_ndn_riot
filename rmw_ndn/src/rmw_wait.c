/*
    rmw_ndn
    Copyright (C) 2017 INRIA

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "rmw/rmw.h"

#include <stdlib.h>

#include <xtimer.h>

#include "app.h"

#define ENABLE_DEBUG 0
#include <debug.h>

rmw_ret_t
rmw_wait(
    rmw_subscriptions_t * subscriptions,
    rmw_guard_conditions_t * guard_conditions,
    rmw_services_t * services,
    rmw_clients_t * clients,
    rmw_wait_set_t * waitset,
    const rmw_time_t * wait_timeout)
{
  (void) subscriptions;
  (void) guard_conditions;
  (void) services;
  (void) clients;
  (void) waitset;
  (void) wait_timeout;
  DEBUG("rmw_wait" "\n");

  const uint32_t begin = xtimer_now_usec();
  
  uint32_t timeout = 0;
  bool disable_timeout = false;
  if(wait_timeout) {
    timeout = wait_timeout->nsec/1000 + wait_timeout->sec*1000000;
  }
  else {
    disable_timeout = true;
  }
  
  const uint32_t end = begin + timeout;

  do {
    app_update();
    thread_yield();

    bool stop = false;

    for(size_t i = 0 ; i < subscriptions->subscriber_count ; i++) {
      sub_t* sub = (sub_t*)subscriptions->subscribers[i];
      if(sub->_data.next) {
        DEBUG("[%i] => %p can take !\n", (int)i, subscriptions->subscribers[i]);
        stop = true;
      }
    }

    if(stop) {
      for(size_t i = 0 ; i < subscriptions->subscriber_count ; i++) {
        sub_t* sub = (sub_t*)subscriptions->subscribers[i];
        if(!sub->_data.next) {
          subscriptions->subscribers[i] = NULL;
        }
      }
      return RMW_RET_OK;
    }

  } while((xtimer_now_usec() < end) || disable_timeout);

  return RMW_RET_TIMEOUT;
}
