// BasePacket.h

#pragma once

#include "DataBucket.h"
#include <string>
#include <vector>
#include <list>
//using namespace std;

//#define _MEMLEAK_TESTING_

static const U8   GlobalNetworkProtocolVersion = 7;

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
   PacketType_Tournament,
   PacketType_Stat,
   PacketType_Num
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
      BasePacket_CommsHandshake,
      BasePacket_RerouteRequest,
      BasePacket_RerouteRequestResponse,
   };
public:
   BasePacket( int packet_type = PacketType_Base, int packet_sub_type = BasePacket_Type ) :
      packetType( packet_type ),
      packetSubType( packet_sub_type ),
      versionNumber( GlobalNetworkProtocolVersion ),
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

class PacketRerouteRequest : public BasePacket
{
public:
   PacketRerouteRequest(): BasePacket( PacketType_Base, BasePacket::BasePacket_RerouteRequest ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////

class PacketRerouteRequestResponse : public BasePacket
{
public:
   enum LocationIds // these will all be gateway instances.. 
   {
      LocationId_Gateway,
      LocationId_Asset,
      LocationId_Login
   };

   PacketRerouteRequestResponse(): BasePacket( PacketType_Base, BasePacket::BasePacket_RerouteRequestResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   struct Address
   {
      string   address;
      string   name;
      U16      port;
      U16      whichLocationId;

      bool  SerializeIn( const U8* data, int& bufferOffset );
      bool  SerializeOut( U8* data, int& bufferOffset ) const;
   };

   SerializedVector< Address > locations;
};


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
      ErrorType_Generic =                                      100,
      ErrorType_Status =                                       101, // most of the time, all we have is status

      ErrorType_Login =                                        202,
      ErrorType_UserBadLogin =                                 203,
      ErrorType_UserDoesNotExist =                             204,
      ErrorType_UserBlocked =                                  205,
      ErrorType_UserLoggedOut =                                206,
      ErrorType_UserNotFinishedWithLogin =                     207, 

      ErrorType_ChatNotCurrentlyAvailable =                    301,// reported at the gateway
      ErrorType_BadChatChannel =                               302,
      ErrorType_NoChatChannel =                                303,
      ErrorType_UserNotOnline =                                304,
      ErrorType_NotAMemberOfThatChatChannel  =                 305,
      ErrorType_YouAreTheOnlyPersonInThatChatChannel =         306,
      ErrorType_CannotAddUserToChannel_AlreadyExists =         307,
      ErrorType_NoChatHistoryExistsOnSelectedChannel =         308,
      ErrorType_NoChatHistoryExistsForThisUser =               309,
      ErrorType_ChatChannelCannotBeCreated =                   310,
      ErrorType_ChatChannelCannotBeDeleted =                   311,
      ErrorType_UserUnknown =                                  312,

      ErrorType_CreateFailed_BadPassword =                     410,
      ErrorType_CreateFailed_DisallowedUsername =              411,
      ErrorType_CreateFailed_DuplicateUsername =               412,
      ErrorType_CreateFailed_DuplicateEmail =                  413,
      ErrorType_CreateFailed_UserCreateAccountPending =        414,
      ErrorType_CreateAccount_Success =                        415,
      ErrorType_CreateAccount_AccountUpdated =                 416,

      ErrorType_Contact_Invitation_success =                   500,
      ErrorType_Contact_Invitation_ProblemFindingUser =        501,
      ErrorType_Contact_Invitation_InviteeAlreadyInvited =     502,
      ErrorType_Contact_Invitation_InviteeAlreadyFriend =      503,
      ErrorType_Contact_Invitation_InvalidUser =               504,
      ErrorType_Contact_Invitation_CannotInviteSelf =          505,
      ErrorType_Contact_Invitation_Accepted =                  506,
      ErrorType_Contact_Invitation_BadInvitation =             507,
      ErrorType_Contact_BadLoginKey =                          508,
      ErrorType_Contact_BadSearchString =                      509,
      ErrorType_Contact_NoSearchResult =                       510,
      ErrorType_Contact_SearchRequestHasTooMany =              511,
      ErrorType_Contact_NotAUserContact =                      512,
      ErrorType_Contact_Invitation_InviteeSentYouInvitation =  513,

      ErrorType_Asset_BadLoginKey =                            600,
      ErrorType_Asset_UnknownAsset =                           601,
      ErrorType_Asset_UserDisconnected =                       602,
      ErrorType_Asset_UnknownAssetCategory =                   603,
      ErrorType_Asset_NoCategoriesAvailable =                  604,
      
      ErrorType_Cheat_BadPermissions =                         700,
      ErrorType_Cheat_BadUserLookup =                          701,
      ErrorType_Cheat_BadUserLookup_TryLoadingUserFirst =      702,
      ErrorType_Cheat_UnrecognizedCommand =                    703,
      ErrorType_Cheat_UnknownError =                           704,
      ErrorType_Cheat_ProductUnknown =                         705,
      ErrorType_Login_CannotAddCurrentProductToUser =          706,
      ErrorType_Login_ProfileIsAlreadyBeingUpdated =           707,

      ErrorType_Purchase_BadPurchaseId =                       800,
      ErrorType_Purchase_StoreBusy =                           801,
      ErrorType_Purchase_UserDoesNotHaveEnoughToTrade =        802,
      ErrorType_Purchase_UserInventoryIsFull =                 803,
      ErrorType_Purchase_ItemNotAvailable =                    804,
      ErrorType_Purchase_TimePeriodHasNotBegunYet =            805,
      ErrorType_Purchase_TimePeriodHasExpired =                806,
      ErrorType_Purchase_UserCannotPurchaseAnyMoreOfThese =    807,
      ErrorType_Purchase_AllPurchasingIsClosedRightNow =       808,
      ErrorType_Purchase_Success =                             809,

      ErrorType_TournamentPurchase_Result_PurchasePending =    900,// you are already waiting for a purchase to complete.
      ErrorType_TournamentPurchase_Result_TooManyPlayers =     901,
      ErrorType_TournamentPurchase_Result_RequirementsNotMet = 902,
      ErrorType_TournamentPurchase_Result_TournamentClosed =   903,
      ErrorType_TournamentPurchase_Result_UnknownUuid =        904,

      ErrorType_Limit =                                      10000
   };

   enum StatusSubtype
   {
      StatusSubtype_ProductAdded = 1,
      StatusSubtype_AllProductsRemoved,
      StatusSubtype_UserIsAdminAccount,

      StatusSubtype_ExtendedPermissionsGiven
   };
public:
   //PacketErrorReport( int packet_sub_type = ErrorType_Generic ): BasePacket( PacketType_ErrorReport ), errorCode( packet_sub_type ), statusInfo( 0 ) {}
   PacketErrorReport( int packet_sub_type = ErrorType_Generic, int subType = 0 ): BasePacket( PacketType_ErrorReport ), errorCode( packet_sub_type ), statusInfo( subType ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U16   errorCode;
   U16   statusInfo;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

#include "BasePacket.inl"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PackageForServerIdentification( const string& serverName, const string& ipAddress, U32 serverId, U16 serverPort, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, bool isGateway, BasePacket** packet );