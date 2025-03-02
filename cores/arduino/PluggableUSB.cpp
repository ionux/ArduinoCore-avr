/*
  PluggableUSB.cpp
  Copyright (c) 2015 Arduino LLC

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "USBAPI.h"
#include "PluggableUSB.h"

#if defined(USBCON)	
#ifdef PLUGGABLE_USB_ENABLED

extern uint8_t _initEndpoints[];

int PluggableUSB_::getInterface(uint8_t* interfaceCount)
{
	int sent = 0;
    int res = 0;
    
	PluggableUSBModule* node = NULL;

    if (rootNode != NULL)
    {
    	for (node = rootNode; node; node = node->next)
        {
    		res = node->getInterface(interfaceCount);
            
    		if (res < 0)
            {
    			sent = -1;
                break;
            }

    		sent += res;
    	}
    }

	return sent;
}

int PluggableUSB_::getDescriptor(USBSetup& setup)
{
    int res = 0;

	PluggableUSBModule* node = NULL;

    if (rootNode != NULL)
    {
    	for (node = rootNode; node; node = node->next)
        {
    		res = node->getDescriptor(setup);

    		// res != 0 means the request has been processed
    		if (res)
            {
    			break;
            }
    	}
    }

	return res;
}

void PluggableUSB_::getShortName(char *iSerialNum)
{
	PluggableUSBModule* node;

	for (node = rootNode; node; node = node->next)
    {
		iSerialNum += node->getShortName(iSerialNum);
	}

	*iSerialNum = 0;
}

bool PluggableUSB_::setup(USBSetup& setup)
{
	PluggableUSBModule* node;

	for (node = rootNode; node; node = node->next)
    {
		if (node->setup(setup))
        {
			return true;
		}
	}

	return false;
}

bool PluggableUSB_::plug(PluggableUSBModule *node)
{
	if ((lastEp + node->numEndpoints) > USB_ENDPOINTS)
    {
		return false;
	}

	if (!rootNode)
    {
		rootNode = node;
	}
    else
    {
		PluggableUSBModule *current = rootNode;

		while (current->next)
        {
			current = current->next;
		}

		current->next = node;
	}

	node->pluggedInterface = lastIf;
	node->pluggedEndpoint = lastEp;

	lastIf += node->numInterfaces;

	for (uint8_t i = 0; i < node->numEndpoints; i++)
    {
		_initEndpoints[lastEp] = node->endpointType[i];

		lastEp++;
	}

	return true;
	// restart USB layer???
}

PluggableUSB_& PluggableUSB()
{
	static PluggableUSB_ obj;

	return obj;
}

PluggableUSB_::PluggableUSB_()
    : lastIf(CDC_ACM_INTERFACE + CDC_INTERFACE_COUNT),
      lastEp(CDC_FIRST_ENDPOINT + CDC_ENPOINT_COUNT),
      rootNode(NULL)
{
	// Intentionally empty.
}

#endif

#endif /* if defined(USBCON) */
