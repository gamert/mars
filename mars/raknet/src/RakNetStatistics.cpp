/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/// \file
///



#include "RakNetStatistics.h"
#include <stdio.h> // sprintf
#include "GetTime.h"
#include "RakString.h"

using namespace RakNet;

// Verbosity level currently supports 0 (low), 1 (medium), 2 (high)
// Buffer must be hold enough to hold the output string.  See the source to get an idea of how many bytes will be output
void RAK_DLL_EXPORT RakNet::StatisticsToString( RakNetStatistics *s, char *buffer, int verbosityLevel )
{
	if ( s == 0 )
	{
		sprintf( buffer, "stats is a NULL pointer in statsToString\n" );
		return ;
	}

	if (verbosityLevel==0)
	{
		sprintf(buffer,
			"BPS sent     %" PRINTF_64_BIT_MODIFIER "u\n"
			"BPS received %" PRINTF_64_BIT_MODIFIER "u\n"
			"Current packetloss        %.1f%%\n",
			(long long unsigned int) s->valueOverLastSecond[ACTUAL_BYTES_SENT],
			(long long unsigned int) s->valueOverLastSecond[ACTUAL_BYTES_RECEIVED],
			s->packetlossLastSecond*100.0f
			);
	}
	else if (verbosityLevel==1)
	{
		sprintf(buffer,
			"Actual BPS sent       %" PRINTF_64_BIT_MODIFIER "u\n"
			"Actual BPS received   %" PRINTF_64_BIT_MODIFIER "u\n"
			"Message BPS pushed    %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total actual bytes sent            %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total actual bytes received        %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total message bytes pushed         %" PRINTF_64_BIT_MODIFIER "u\n"
			"Current packetloss                 %.1f%%\n"
			"Average packetloss                 %.1f%%\n"
			"Elapsed connection time in seconds %" PRINTF_64_BIT_MODIFIER "u\n",
			(long long unsigned int) s->valueOverLastSecond[ACTUAL_BYTES_SENT],
			(long long unsigned int) s->valueOverLastSecond[ACTUAL_BYTES_RECEIVED],
			(long long unsigned int) s->valueOverLastSecond[USER_MESSAGE_BYTES_PUSHED],
			(long long unsigned int) s->runningTotal[ACTUAL_BYTES_SENT],
			(long long unsigned int) s->runningTotal[ACTUAL_BYTES_RECEIVED],
			(long long unsigned int) s->runningTotal[USER_MESSAGE_BYTES_PUSHED],
			s->packetlossLastSecond*100.0f,
			s->packetlossTotal*100.0f,
			(long long unsigned int) (uint64_t)((RakNet::GetTimeUS()-s->connectionStartTime)/1000000)
			);

		if (s->BPSLimitByCongestionControl!=0)
		{
			char buff2[128];
			sprintf(buff2,
				"Send capacity                    %" PRINTF_64_BIT_MODIFIER "u BPS (%.0f%%)\n",
				(long long unsigned int) s->BPSLimitByCongestionControl,
				100.0f * s->valueOverLastSecond[ACTUAL_BYTES_SENT] / s->BPSLimitByCongestionControl
				);
			strcat(buffer,buff2);
		}
		if (s->BPSLimitByOutgoingBandwidthLimit!=0)
		{
			char buff2[128];
			sprintf(buff2,
				"Send limit                       %" PRINTF_64_BIT_MODIFIER "u (%.0f%%)\n",
				(long long unsigned int) s->BPSLimitByOutgoingBandwidthLimit,
				100.0f * s->valueOverLastSecond[ACTUAL_BYTES_SENT] / s->BPSLimitByOutgoingBandwidthLimit
				);
			strcat(buffer,buff2);
		}
	}	
	else
	{
		sprintf(buffer,
			"Actual BPS sent         %" PRINTF_64_BIT_MODIFIER "u\n"
			"Actual BPS received     %" PRINTF_64_BIT_MODIFIER "u\n"
			"Message BPS sent        %" PRINTF_64_BIT_MODIFIER "u\n"
			"Message BPS resent      %" PRINTF_64_BIT_MODIFIER "u\n"
			"Message BPS pushed      %" PRINTF_64_BIT_MODIFIER "u\n"
			"Message BPS returned	 %" PRINTF_64_BIT_MODIFIER "u\n"
			"Message BPS ignored     %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total sent				 %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total received          %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total message sent      %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total message resent    %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total message pushed    %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total message returned	 %" PRINTF_64_BIT_MODIFIER "u\n"
			"Total message ignored   %" PRINTF_64_BIT_MODIFIER "u\n"
			"Messages in send buffer, %i,%i,%i,%i\n"
			"Bytes in send buffer,	  %i,%i,%i,%i\n"
			"Messages in resend buffer	%i\n"
			"Bytes in resend buffer  %" PRINTF_64_BIT_MODIFIER "u\n"
			"Current packetloss      %.1f%%\n"
			"Average packetloss      %.1f%%\n"
			"Elapsed connection time in seconds   %" PRINTF_64_BIT_MODIFIER "u\n"
			"NAKs            %i\n",
			(long long unsigned int) s->valueOverLastSecond[ACTUAL_BYTES_SENT],
			(long long unsigned int) s->valueOverLastSecond[ACTUAL_BYTES_RECEIVED],
			(long long unsigned int) s->valueOverLastSecond[USER_MESSAGE_BYTES_SENT],
			(long long unsigned int) s->valueOverLastSecond[USER_MESSAGE_BYTES_RESENT],
			(long long unsigned int) s->valueOverLastSecond[USER_MESSAGE_BYTES_PUSHED],
			(long long unsigned int) s->valueOverLastSecond[USER_MESSAGE_BYTES_RECEIVED_PROCESSED],
			(long long unsigned int) s->valueOverLastSecond[USER_MESSAGE_BYTES_RECEIVED_IGNORED],
			(long long unsigned int) s->runningTotal[ACTUAL_BYTES_SENT],
			(long long unsigned int) s->runningTotal[ACTUAL_BYTES_RECEIVED],
			(long long unsigned int) s->runningTotal[USER_MESSAGE_BYTES_SENT],
			(long long unsigned int) s->runningTotal[USER_MESSAGE_BYTES_RESENT],
			(long long unsigned int) s->runningTotal[USER_MESSAGE_BYTES_PUSHED],
			(long long unsigned int) s->runningTotal[USER_MESSAGE_BYTES_RECEIVED_PROCESSED],
			(long long unsigned int) s->runningTotal[USER_MESSAGE_BYTES_RECEIVED_IGNORED],
			s->messageInSendBuffer[IMMEDIATE_PRIORITY],s->messageInSendBuffer[HIGH_PRIORITY],s->messageInSendBuffer[MEDIUM_PRIORITY],s->messageInSendBuffer[LOW_PRIORITY],
			(unsigned int) s->bytesInSendBuffer[IMMEDIATE_PRIORITY],(unsigned int) s->bytesInSendBuffer[HIGH_PRIORITY],(unsigned int) s->bytesInSendBuffer[MEDIUM_PRIORITY],(unsigned int) s->bytesInSendBuffer[LOW_PRIORITY],
			s->messagesInResendBuffer,
			(long long unsigned int) s->bytesInResendBuffer,
			s->packetlossLastSecond*100.0f,
			s->packetlossTotal*100.0f,
			(long long unsigned int) (uint64_t)((RakNet::GetTimeUS()-s->connectionStartTime)/1000000),
			s->NAK_RECEIVED
			);

		if (s->BPSLimitByCongestionControl!=0)
		{
			char buff2[128];
			sprintf(buff2,
				"Send capacity                    %" PRINTF_64_BIT_MODIFIER "u BPS (%.0f%%)\n",
				(long long unsigned int) s->BPSLimitByCongestionControl,
				100.0f * s->valueOverLastSecond[ACTUAL_BYTES_SENT] / s->BPSLimitByCongestionControl
				);
			strcat(buffer,buff2);
		}
		if (s->BPSLimitByOutgoingBandwidthLimit!=0)
		{
			char buff2[128];
			sprintf(buff2,
				"Send limit                       %" PRINTF_64_BIT_MODIFIER "u (%.0f%%)\n",
				(long long unsigned int) s->BPSLimitByOutgoingBandwidthLimit,
				100.0f * s->valueOverLastSecond[ACTUAL_BYTES_SENT] / s->BPSLimitByOutgoingBandwidthLimit
				);
			strcat(buffer,buff2);
		}
	}
}
