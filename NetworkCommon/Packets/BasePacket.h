// BasePacket.h

#pragma once

#include "DataBucket.h"
#include <string>
#include <vector>
#include <list>
//using namespace std;

//#define _MEMLEAK_TESTING_

///////////////////////////////////////////////////////////////

// todo, make login, logout, etc into authentication packets
enum PacketType
{
   PacketType_Base,
   PacketType_Login,
   PacketType_Chat,
   PacketType_UserInfo,
   PacketType_Contact,
   PacketType_Asset,
   PacketType_UserStateChange, // from server to client, usually
   PacketType_DbQuery,
   PacketType_Gameplay,
   PacketType_GatewayWrapper,
   PacketType_ServerToServerWrapper,
   PacketType_ServerJobWrapper,
   PacketType_ServerInformation,
   PacketType_GatewayInformation, // user logged out, prepare to shutdown, etc.
   PacketType_ErrorReport,
   PacketType_Cheat,
   PacketType_Purchase,
   PacketType_Tournament
};


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//#define _MEMORY_TEST_
#ifdef _MEMORY_TEST_
#include <iostream>
using namespace std;
#endif

class BasePacket
{
public:
   enum SubType 
   {
      BasePacket_Type,
      BasePacket_Hello,
      BasePacket_CommsHandshake
   };
public:
   BasePacket( int packet_type = PacketType_Base, int packet_sub_type = BasePacket_Type ) :
      packetType( packet_type ),
      packetSubType( packet_sub_type ),
      versionNumber( 0 ),
      gameInstanceId( 0 )
      {
#ifdef _MEMORY_TEST_
m_counter++;
cout << "BasePacket +count: " << m_counter << endl;
#endif
      }
   virtual ~BasePacket()
   {
#ifdef _MEMORY_TEST_
m_counter--;
cout << "BasePacket ~count: " << m_counter << endl;
#endif
      gameInstanceId = 0;// for a place to set breakpoints.
   }

   virtual bool  SerializeIn( const U8* data, int& bufferOffset );
   virtual bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U8       packetType;
   U8       packetSubType;
   U8       versionNumber;
   U8       gameProductId;
   U32      gameInstanceId;
#ifdef _MEMORY_TEST_
   static int      m_counter;
#endif
};

///////////////////////////////////////////////////////////////

class PacketHello : public BasePacket
{
public:
   PacketHello(): BasePacket( PacketType_Base, BasePacket::BasePacket_Hello ) {}
   // serialize by base is good enough
};

///////////////////////////////////////////////////////////////

class PacketCommsHandshake : public BasePacket
{
public:
   PacketCommsHandshake(): BasePacket( PacketType_Base, BasePacket::BasePacket_CommsHandshake ), serverHashedKey( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32   serverHashedKey;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////


class ChannelInfo
{
public:
   ChannelInfo() {}
   ChannelInfo( const string& name, const string& uuid, int gameProductId, int _gameId, int _numNewChats, bool active) : 
               channelName( name ), channelUuid( uuid ), gameProduct( gameProductId ), gameId( _gameId ), numNewChats( _numNewChats ), isActive( active ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   channelName;
   string   channelUuid;
   int      gameProduct;
   int      gameId;
   int      numNewChats;
   bool     isActive;
};

typedef SerializedKeyValueVector< ChannelInfo > ChannelKeyValue;

///////////////////////////////////////////////////////////////////


class PacketUserInfo : public BasePacket
{
public:
   enum InfoType
   {
      InfoType_None,
      InfoType_FriendsListRequest,
      InfoType_FriendsList,
      InfoType_GroupsListRequest,
      InfoType_GroupsList,
      InfoType_ChatChannelListRequest,
      InfoType_ChatChannelList
   };
public:
   PacketUserInfo( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_None ) : BasePacket( packet_type, packet_sub_type ){  }
};

///////////////////////////////////////////////////////////////

class PacketFriendsListRequest : public PacketUserInfo
{
public:
   PacketFriendsListRequest( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_FriendsListRequest ) : PacketUserInfo( packet_type, packet_sub_type ){  }
};


///////////////////////////////////////////////////////////////

class PacketFriendsList : public PacketUserInfo
{
public:
   PacketFriendsList( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_FriendsList ) : PacketUserInfo( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< string > friendList;
};

///////////////////////////////////////////////////////////////

class PacketGroupsListRequest : public PacketUserInfo
{
public:
   PacketGroupsListRequest( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_GroupsListRequest ) : PacketUserInfo( packet_type, packet_sub_type ){  }
};


///////////////////////////////////////////////////////////////

class PacketGroupsList : public PacketUserInfo
{
public:
   PacketGroupsList( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_GroupsList ) : PacketUserInfo( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< ChannelInfo >  groupList;
};


///////////////////////////////////////////////////////////////

class PacketChatChannelListRequest : public PacketUserInfo
{
public:
   PacketChatChannelListRequest( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_ChatChannelListRequest ) : PacketUserInfo( packet_type, packet_sub_type ){  }
};


///////////////////////////////////////////////////////////////

class PacketChatChannelList : public PacketUserInfo
{
public:
   PacketChatChannelList( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_ChatChannelList ) : PacketUserInfo( packet_type, packet_sub_type ){  }
   ~PacketChatChannelList();

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< ChannelInfo >  channelList;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

// todo, fill in the details
class PacketUserStateChange : public BasePacket
{
public:
   enum StateChangeType
   {
      StateChangeType_None,
      StateChangeType_Rename,
      StateChangeType_Login,
      StateChangeType_Logout,
      StateChangeType_JoinChannel,
      StateChangeType_LeaveChannel,
   };
public:
   PacketUserStateChange( int packet_type = PacketType_UserStateChange, int packet_sub_type = StateChangeType_None ): BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   string   username;
};

class PacketUserStateChange_Login : public PacketUserStateChange
{
public:
   PacketUserStateChange_Login(): PacketUserStateChange( PacketType_UserStateChange, StateChangeType_Login ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return PacketUserStateChange::SerializeIn( data, bufferOffset ); }
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return PacketUserStateChange::SerializeOut( data, bufferOffset ); }

   string   uuid;
   string   username;
};

class PacketUserStateChange_Logout : public PacketUserStateChange
{
public:
   PacketUserStateChange_Logout(): PacketUserStateChange( PacketType_UserStateChange, StateChangeType_Logout ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return PacketUserStateChange::SerializeIn( data, bufferOffset ); }
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return PacketUserStateChange::SerializeOut( data, bufferOffset ); }

   string   uuid;
   string   username;
};

///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketGatewayWrapper : public BasePacket
{
public:
   PacketGatewayWrapper( int packet_type = PacketType_GatewayWrapper, int packet_sub_type = 0  ): BasePacket( packet_type, packet_sub_type ), connectionId( -1 ), size( 0 ), pPacket( NULL ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool  HeaderSerializeIn( const U8* data, int bufferOffset );// NOTE: we pass by value here because we do not want to modify variables passed in.

   enum { BufferSize=12*1024 };
   U32         connectionId;
   U16         size;
   BasePacket* pPacket;

   void  SetupPacket( BasePacket* packet, U32 connectionId );

protected:
   static U8 SerializeBuffer [BufferSize];
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////


class PacketErrorReport : public BasePacket
{
public:
   enum ErrorType
   {
      ErrorType_Generic,
      ErrorType_Status, // most of the time, all we have is status
      ErrorType_Login,

      ErrorType_UserBadLogin,
      ErrorType_UserDoesNotExist,
      ErrorType_UserBlocked,
      ErrorType_UserLoggedOut, //6

      ErrorType_ChatNotCurrentlyAvailable,// reported at the gateway
      ErrorType_BadChatChannel,
      ErrorType_NoChatChannel,
      ErrorType_UserNotOnline,
      ErrorType_NotAMemberOfThatChatChannel,
      ErrorType_YouAreTheOnlyPersonInThatChatChannel,
      ErrorType_CannotAddUserToChannel_AlreadyExists,
      ErrorType_NoChatHistoryExistsOnSelectedChannel,
      ErrorType_NoChatHistoryExistsForThisUser, // 16

      ErrorType_CreateFailed_BadPassword,
      ErrorType_CreateFailed_DisallowedUsername,
      ErrorType_CreateFailed_DuplicateUsername,
      ErrorType_CreateFailed_DuplicateEmail,
      ErrorType_CreateFailed_UserCreateAccountPending,
      ErrorType_CreateAccount_Success,
      ErrorType_CreateAccount_AccountUpdated, // 23

      ErrorType_Contact_Invitation_success, // 24
      ErrorType_Contact_Invitation_ProblemFindingUser,
      ErrorType_Contact_Invitation_InviteeAlreadyInvited,
      ErrorType_Contact_Invitation_InviteeAlreadyFriend,
      ErrorType_Contact_Invitation_InvalidUser,
      ErrorType_Contact_Invitation_CannotInviteSelf,
      ErrorType_Contact_Invitation_Accepted,
      ErrorType_Contact_Invitation_BadInvitation,
      ErrorType_Contact_BadLoginKey,

      ErrorType_Asset_BadLoginKey,
      
      ErrorType_Cheat_BadPermissions,
      ErrorType_Cheat_BadUserLookup,
      ErrorType_Cheat_BadUserLookup_TryLoadingUserFirst,
      ErrorType_Cheat_UnrecognizedCommand,
      ErrorType_Cheat_UnknownError,
      ErrorType_Cheat_ProductUnknown,
      ErrorType_Login_CannotAddCurrentProductToUser,
      ErrorType_Login_ProfileIsAlreadyBeingUpdated,

      ErrorType_Purchase_BadPurchaseId,
      ErrorType_Purchase_StoreBusy,
      ErrorType_Purchase_UserDoesNotHaveEnoughToTrade,
      ErrorType_Purchase_UserInventoryIsFull,
      ErrorType_Purchase_ItemNotAvailable,
      ErrorType_Purchase_TimePeriodHasNotBegunYet,
      ErrorType_Purchase_TimePeriodHasExpired,
      ErrorType_Purchase_UserCannotPurchaseAnyMoreOfThese,
      ErrorType_Purchase_AllPurchasingIsClosedRightNow,
      ErrorType_Purchase_Success

   };

   enum StatusSubtype
   {
      StatusSubtype_ProductAdded = 1,
      StatusSubtype_AllProductsRemoved,
      StatusSubtype_UserIsAdminAccount,

      StatusSubtype_ExtendedPermissionsGiven
   };
public:
   PacketErrorReport( int packet_sub_type = ErrorType_Generic ): BasePacket( PacketType_ErrorReport, packet_sub_type ), statusInfo( 0 ) {}
   PacketErrorReport( int packet_sub_type, int subType = 0 ): BasePacket( PacketType_ErrorReport, packet_sub_type ), statusInfo( subType ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   int   statusInfo;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

#include "BasePacket.inl"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PackageForServerIdentification( const string& serverName, U32 serverId, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, bool isGateway, BasePacket** packet );