/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/
#include "BankTerminal.h"
#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Item.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "PlayerObject.h"
#include "SurveyTool.h"
#include "TravelMapHandler.h"
#include "UIManager.h"
#include "Wearable.h"
#include "WorldConfig.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

#include <boost/lexical_cast.hpp>

//=============================================================================
//
// chat
//
void ObjectController::_handleSpatialChatInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// FIXME: for now assume only players send chat
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	string			chatData;


	message->getStringUnicode16(chatData);
	chatData.convert(BSTRType_ANSI);

	int8* data = chatData.getRawData();
	uint16 len = chatData.getLength();

	char chatElement[5][32];

	uint8 element		= 0;
	uint8 elementIndex	= 0;
	uint16 byteCount	= 0;

	while(element < 5)
	{
		if(*data == ' ')
		{
			chatElement[element][elementIndex] = 0;
			byteCount++;
			element++;
			data++;
			elementIndex = 0;
			continue;
		}

		chatElement[element][elementIndex] = *data;
		elementIndex++;
		byteCount++;
		data++;
	}

	string chatMessage(data);

	// need to truncate or we may get in trouble
	if(len - byteCount > 256)
	{
		chatMessage.setLength(256);
		chatMessage.getRawData()[256] = 0;
	}
	else
	{
		chatMessage.setLength(len - byteCount);
		chatMessage.getRawData()[len - byteCount] = 0;
	}

	chatMessage.convert(BSTRType_Unicode16);

	if (!gWorldConfig->isInstance())
	{
		gMessageLib->sendSpatialChat(playerObject,chatMessage,chatElement);
	}
	else
	{
		gMessageLib->sendSpatialChat(playerObject,chatMessage,chatElement, playerObject);
	}
}

//=============================================================================
//
// emote
//

void ObjectController::_handleSocialInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// FIXME: for now assume only players send chat
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	string			emoteData;
	BStringVector	emoteElement;

	message->getStringUnicode16(emoteData);

	// Have to convert BEFORE using split, since the conversion done there is removed It will assert().. evil grin...
	// Either do the conversion HERE, or better fix the split so it handles unicoe also.
	emoteData.convert(BSTRType_ANSI);
	emoteData.split(emoteElement,' ');

	uint64 emoteTarget = boost::lexical_cast<uint64>(emoteElement[0].getAnsi());
	uint16 emoteId		= atoi(emoteElement[1].getAnsi());
	uint16 sendType		= 0x0100;

	// if set, send text along with animation
	if(atoi(emoteElement[3].getAnsi()) == 1)
		sendType = 0x0300;

	gMessageLib->sendSpatialEmote(playerObject,emoteId,sendType,emoteTarget);
}

//=============================================================================
//
// mood
//

void ObjectController::_handleSetMoodInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// FIXME: for now assume only players send chat
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	string			moodStr;
	int8			sql[256];

	message->getStringUnicode16(moodStr);
	moodStr.convert(BSTRType_ANSI);
	uint32 mood = boost::lexical_cast<uint32>(moodStr.getAnsi());

	playerObject->setMoodId(static_cast<uint8>(mood));

	gMessageLib->sendMoodUpdate(playerObject);

	ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_Nope);
	sprintf(sql,"UPDATE swganh.character_attributes SET moodId = %u where character_id = %"PRIu64"",mood,playerObject->getId());

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

}

//=============================================================================


