//IBM_PROLOG_BEGIN_TAG
/* 
 * Copyright 2003,2023 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//IBM_PROLOG_END_TAG

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include "adal_base.h"

bool adal_is_byte_swap_needed( ) {
    struct utsname unameBuf;

    char *str, *token, *prvToken, *saveptr;

    int rc = uname(&unameBuf);
    if ( rc ) return false;

    // idx 1 < 4 return false, version is less than 4.10
    // idx 1 > 4 return true, version is greater than 4.10
    // idx 2 < 10 and idx 1 == 4 return false, version is less than 4.10
    // idx 2 >= 10 and idx 1 == 4 return true, version is greater than 4.10
    str = unameBuf.release;
    for( int idx=1; ; idx++, str=NULL )
    {
        token = strtok_r( str, ".", &saveptr );
        if ( token == NULL )
            break;
        if ( idx == 1 && atoi(token) < 4 )
            return false;
        else if ( idx == 1 && atoi(token) > 4 )
            return true;
        else if ( idx == 2 && atoi(token) < 10 && atoi(prvToken) == 4 )
            return false;
        else if ( idx == 2 && atoi(token) >= 10 && atoi(prvToken) == 4 )
            return true;
        else if ( idx > 2 )
            break;
        prvToken = token;
    }
    return false;
}

adal_t * adal_base_open(const char* i_device, int flags)
{
    adal_t * adal = NULL;

	adal = (adal_t *)malloc(sizeof(*adal));
	if (adal == NULL) {
		return NULL;
	}

	adal->fd = open(i_device, flags);
	if (adal->fd == -1) {
		free(adal);
		adal = NULL;
	}
	if (adal != NULL) {
		/* store away file string for /sys access later
		   use strndup for reasonable limit on path name (128 char) */
		adal->priv = strndup(i_device, 128);
	}

	return adal;
}

int adal_base_close(adal_t * i_adal)
{
    int rc = 0;

    if( !i_adal ) 
        return 0;

    free(i_adal->priv);
    i_adal->priv = NULL;
    rc = close(i_adal->fd);

    free(i_adal);
    i_adal = NULL;

    return rc;
}

const uint32_t fsiRawDevicesNum = 9;
const uint32_t fsiRawDevicesNumPaths = 5;
const char * fsiRawDevices[9][5] = {
    {"", "", "", "", ""},
    {"/sys/class/fsi-master/fsi0/slave@00:00/raw",
     "/sys/devices/platform/gpio-fsi/fsi0/slave@00:00/raw",
     "/sys/devices/platform/fsi-master/slave@00:00/raw",
     "/sys/bus/platform/devices/fsi-master/slave@00:00/raw",
     ""},
    {"/sys/class/fsi-master/fsi1/slave@01:00/raw",
     "/sys/class/fsi-master/fsi0/slave@00:00/00:00:00:0a/fsi-master/fsi1/slave@01:00/raw",
     "/sys/devices/platform/gpio-fsi/fsi0/slave@00:00/00:00:00:0a/fsi1/slave@01:00/raw",
     "/sys/devices/platform/fsi-master/slave@00:00/hub@00/slave@01:00/raw",
     "/sys/devices/hub@00/slave@01:00/raw"},
    {"/sys/class/fsi-master/fsi1/slave@02:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@03:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@04:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@05:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@06:00/raw", "", "", "", ""},
    {"/sys/class/fsi-master/fsi1/slave@07:00/raw", "", "", "", ""}
};

adal_t * adal_fsi_open(const uint32_t i_device, int i_flags)
{
    adal_t * l_adal = NULL;

    // deviceNum must be between 1 and 8.
    if( i_device < 1 || i_device > 8 )
    {
        return NULL;
    }

    // try all the devices in the fsiRawDevices list (backwards compatibility stuff)
    for( uint32_t l_numPathsIdx = 0; l_numPathsIdx < fsiRawDevicesNumPaths; l_numPathsIdx++ )
    {

#ifdef TESTING
        TEST_PRINT("adal_base_open(%s, %d)\n", fsiRawDevices[i_deviceNum][l_numPathsIdx], i_flags);
        *l_adal = (adal_t *) 0x1;
#else
        l_adal = adal_base_open(fsiRawDevices[i_device][l_numPathsIdx], i_flags);
#endif
        if( l_adal != NULL )
        {
            // successful open of the device
            break;
        }
        else
        {
            continue;
        }
    }

    return l_adal;
}