// ChatUser.cpp

#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "ChatUser.h"
#include "diplodocusChat.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
//#include "ChatChannelManager.h"

#include <boost/lexical_cast.hpp>

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusChat*      ChatUser::  m_chatServer = NULL;
ChatChannelManager*  ChatUser::  m_chatChannelManager = NULL;

void ChatUser::Set( DiplodocusChat* chat )
{
   m_chatServer = chat;
}

void ChatUser::Set( ChatChannelManager* mgr )
{
   m_chatChannelManager = mgr;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

ChatUser::ChatUser( U32 connectionId ) : m_userId ( 0 ), 
                                          m_connectionId( connectionId ), 
                                          m_pendingQueries( QueryType_All ),
                                          m_isLoggedIn( false )
{
}

//---------------------------------------------------------

ChatUser::~ChatUser()
{
}

//---------------------------------------------------------

void  ChatUser::Init( U32 userId, const string& name, const string& uuid, const string& lastLoginTime )
{
   m_userName =      name;
   m_uuid =          uuid;
   m_lastLoginTime = lastLoginTime;
   m_userId =      userId;
}

//---------------------------------------------------------

void     ChatUser::LoggedOut() 
{ 
   m_isLoggedIn = false; 
   time( &m_loggedOutTime );
}


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////