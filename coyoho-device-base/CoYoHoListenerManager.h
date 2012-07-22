/**
 * This file is part of the CoYoHo Control Your Home System.
 *
 * Copyright 2011-2012 Dirk Grappendorf, www.grappendorf.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COYOHO_LISTENER_MANAGER_H_
#define COYOHO_LISTENER_MANAGER_H_

#include <XXBee/XXBee.h>
#include <CoYoHoMessages.h>

#define COYOHO_LISTENER_LEASE_TIME (10L * 60L * 1000L)

template<int MAX_LISTENERS> class ListenerManager
{
	public:

	ListenerManager(class XBee * xbee)
	: xbee(xbee)
	{
		for (uint8_t i = 0; i < MAX_LISTENERS; ++i)
		{
			listener[i] = XBeeAddress64(0, 0);
		}
	}

	void addListener(XBeeAddress64 address)
	{
		bool notifyListener = false;
		uint8_t freeListener = MAX_LISTENERS;
		uint8_t i = 0;
		for (; i < MAX_LISTENERS; ++i)
		{
			if (listener[i].getMsb() == 0 && listener[i].getLsb() == 0 && freeListener == MAX_LISTENERS)
			{
				freeListener = i;
			}
			else if (address.getMsb() == listener[i].getMsb() && address.getLsb() == listener[i].getLsb())
			{
				notifyListener = true;
				break;
			}
		}
		if (i == MAX_LISTENERS && freeListener < MAX_LISTENERS)
		{
			notifyListener = true;
			i = freeListener;
			listener[i] = address;
		}
		if (notifyListener)
		{
			uint8_t message[] =
			{	COYOHO_MESSAGE_RESPONSE | COYOHO_ADD_LISTENER};
			ZBTxRequest txRequest(address, message, sizeof(message));
			xbee->send(txRequest);
			listenerLeaseTimeout[i] = millis() + COYOHO_LISTENER_LEASE_TIME;
		}
	}

	void removeListener(XBeeAddress64 address)
	{
		uint8_t i = 0;
		for (; i < MAX_LISTENERS; ++i)
		{
			if (address.getMsb() == listener[i].getMsb() && address.getLsb() == listener[i].getLsb())
			{
				listener[i] = XBeeAddress64(0, 0);
				listenerLeaseTimeout[i] = 0;
				break;
			}
		}
		if (i != MAX_LISTENERS)
		{
			uint8_t message[] =
			{	COYOHO_MESSAGE_RESPONSE | COYOHO_REMOVE_LISTENER};
			ZBTxRequest txRequest(address, message, sizeof(message));
			xbee->send(txRequest);
		}
	}

	void checkListenerLeases()
	{
		for (uint8_t i = 0; i < MAX_LISTENERS; ++i)
		{
			if (listener[i].getMsb() != 0 && listener[i].getLsb() != 0 && millis() > listenerLeaseTimeout[i])
			{
				listener[i] = XBeeAddress64(0, 0);
			}
		}
	}

	void notifyListeners(uint8_t *message, uint8_t messageSize)
	{
		for (uint8_t i = 0; i < MAX_LISTENERS; ++i)
		{
			if (listener[i].getMsb() != 0 && listener[i].getLsb() != 0)
			{
				ZBTxRequest txRequest(listener[i], message, messageSize);
				xbee->send(txRequest);
			}
		}
	}

	uint8_t listenersCount()
	{
		return MAX_LISTENERS;
	}

	const XBeeAddress64* listenerAddresses()
	{
		return listener;
	}

	bool processXBeeMessage(uint8_t command, XBee &xbee, ZBRxResponse rxResponse)
	{
		switch(command)
		{
			case COYOHO_ADD_LISTENER:
				addListener(rxResponse.getRemoteAddress64());
				return true;

			case COYOHO_REMOVE_LISTENER:
				removeListener(rxResponse.getRemoteAddress64());
				return true;

			default:
				return false;
		}
	}

	private:

	class XBee * xbee;

	XBeeAddress64 listener[MAX_LISTENERS];

	unsigned long listenerLeaseTimeout[MAX_LISTENERS];

	unsigned long nextListenerNotifyMillis;
};

#endif /* COYOHO_LISTENER_MANAGER_H_ */
