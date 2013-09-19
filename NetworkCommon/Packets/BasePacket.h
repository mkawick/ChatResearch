// BasePacket.h

#pragma once

#include "../ServerType.h"
#include "../DataTypes.h"
#include "../ServerConstants.h"
#include "../Serialize.h"
#include <string>
#include <vector>
#include <list>
//using namespace std;

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
   PacketType_Cheat
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class StringBucket
{
public:
   typedef list< string >  DataSet;

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   void  operator = ( const list< string >& copyData );
   void  insert( const string& str ) { bucket.push_back( str ); }
   DataSet  bucket;   
};

///////////////////////////////////////////////////////////////

class DataRow : public vector< string >  
{
public:
   const_reference operator[](size_type _Pos) const{
      const_reference ref = vector< string >::operator []( _Pos );
      if( ref == "NULL")
         return empty;
      else 
         return ref;
   }
   reference operator[](size_type _Pos){
      reference ref = vector< string >::operator []( _Pos );
      if( ref == "NULL")
         return empty;
      else 
         return ref;
   };
   string empty;
};


class DynamicDataBucket
{
public:
   typedef list< DataRow >  DataSet;

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   void  operator = ( const list< DataRow >& copyData );
   void  operator = ( const list< list<string> >& copyData );
   DataSet  bucket;   
};

///////////////////////////////////////////////////////////////

template < typename type = string >
class KeyValueSerializer
{
public:
   KeyValueSerializer(){}
   KeyValueSerializer( string _key, type _value ): key( _key ), value( _value ){}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   key;
   type     value;
};

typedef KeyValueSerializer< string >            KeyValueString;
typedef vector< KeyValueString >                KeyValueVector;
typedef KeyValueVector::iterator                KeyValueVectorIterator;
typedef KeyValueVector::const_iterator          KeyValueConstIterator;

///////////////////////////////////////////////////////////////

template < typename type = string >
class SerializedKeyValueVector
{
public:
   typedef  KeyValueSerializer< type >           KeyValue;
   typedef  vector< KeyValue >                   KeyValueVector;
   typedef typename KeyValueVector::iterator             KeyValueVectorIterator;
   typedef typename KeyValueVector::iterator             KVIterator;
   typedef typename KeyValueVector::const_iterator       const_KVIterator;

   SerializedKeyValueVector() {}
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   const KeyValueVector&   GetData() const { return dataList; }
   void  clear() { dataList.clear(); }
   
   // helper functions
   void  insert( const string& key, const type& obj ) { dataList.push_back( KeyValue( key, obj ) ); }

   int                  size() const { return static_cast< int >( dataList.size() ); }
   const_KVIterator     begin() const { return dataList.begin(); }
   KVIterator           begin() { return dataList.begin(); }
   const_KVIterator     end() const { return dataList.end(); }
   KVIterator           end() { return dataList.end(); }

   bool operator = (const KeyValueSerializer< type >& src );
   bool operator = (const KeyValueVector& src );

protected:
   KeyValueVector    dataList;
};

///////////////////////////////////////////////////////////////

template < typename type >
class SerializedVector
{
public:
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   // helper functions
   void           push_back( type value ) { m_data.push_back( value ); }
   int            size() const { return static_cast< int >( m_data.size() ); }
   const type&    operator[]( int index ) { return m_data[ index ]; }
   const type&    operator[]( int index ) const { return m_data[ index ]; }

protected:
   vector< type >    m_data;
};

///////////////////////////////////////////////////////////////
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
   ~BasePacket()
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

class PacketLogin : public BasePacket
{
public:
   enum LoginType
   {
      LoginType_Login,
      LoginType_Logout,
      LoginType_PacketLogoutToClient,
      LoginType_InformClientOfLoginStatus,
      LoginType_InformGatewayOfLoginStatus,
      LoginType_PrepareForUserLogin,
      LoginType_PrepareForUserLogout,
      LoginType_CreateAccount,
      LoginType_CreateAccountResponse,
      LoginType_RequestListOfPurchases,
      LoginType_ListOfPurchases,
      LoginType_ListOfProductsS2S,
      LoginType_ListOfPurchases_Cheat
   };
public:
   PacketLogin( int packet_type = PacketType_Login, int packet_sub_type = LoginType_Login ): BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
   string   username;
   
   string   loginKey;
   string   password;
   string   languageCode;
};

///////////////////////////////////////////////////////////////

class PacketLogout : public BasePacket
{
public:
   PacketLogout(): BasePacket( PacketType_Login, PacketLogin::LoginType_Logout ), wasDisconnectedByError( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   bool  wasDisconnectedByError;
};

///////////////////////////////////////////////////////////////

class PacketLogoutToClient : public PacketLogin
{
public:
   PacketLogoutToClient(): PacketLogin( PacketType_Login, PacketLogin::LoginType_PacketLogoutToClient ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return PacketLogin::SerializeIn( data, bufferOffset ); }
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return PacketLogin::SerializeOut( data, bufferOffset ); }
};


///////////////////////////////////////////////////////////////

class PacketLoginToClient : public PacketLogin
{
public:
   PacketLoginToClient(): PacketLogin( PacketType_Login, PacketLogin::LoginType_InformClientOfLoginStatus ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   lastLogoutTime;
   bool     wasLoginSuccessful;
   U32      connectionId;  // used for a user ID.
};


///////////////////////////////////////////////////////////////

class PacketLoginToGateway : public PacketLogin
{
public:
   PacketLoginToGateway(): PacketLogin( PacketType_Login, PacketLogin::LoginType_InformGatewayOfLoginStatus ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   lastLogoutTime;
   bool     wasLoginSuccessful;
   U8       adminLevel;
};

///////////////////////////////////////////////////////////////

class PacketCreateAccount : public BasePacket
{
public:
   PacketCreateAccount(): BasePacket( PacketType_Login, PacketLogin::LoginType_CreateAccount ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   string username;
   string useremail;
   string password;
   string deviceId;
   string deviceAccountId;
   U8     languageId;
};

///////////////////////////////////////////////////////////////

class PacketCreateAccountResponse : public BasePacket
{
public:
   PacketCreateAccountResponse(): BasePacket( PacketType_Login, PacketLogin::LoginType_CreateAccountResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   string   username;
   string   useremail;
   bool     wasSuccessful;
};

///////////////////////////////////////////////////////////////

struct PurchaseEntry
{
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   productStoreId; // apple's name for it.
   string   name;
   string   price;
   double   number_price;
   string   date; // not currently available
};

//--------------------------------

class PacketRequestListOfUserPurchases : public BasePacket
{
public:
   PacketRequestListOfUserPurchases(): BasePacket( PacketType_Login, PacketLogin::LoginType_RequestListOfPurchases ) {}
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   int   platformId;
   bool  requestUserOnly;
};

//--------------------------------

class PacketListOfUserPurchases : public BasePacket
{
public:
   PacketListOfUserPurchases(): BasePacket( PacketType_Login, PacketLogin::LoginType_ListOfPurchases ), isAllProducts( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   int   platformId;
   bool  isAllProducts;
   SerializedVector< PurchaseEntry > purchases;
};

//--------------------------------

class PacketListOfUserProductsS2S : public BasePacket
{
public:
   PacketListOfUserProductsS2S(): BasePacket( PacketType_Login, PacketLogin::LoginType_ListOfProductsS2S ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   
   string         uuid;
   U32            connectionId;
   StringBucket   products;
};


///////////////////////////////////////////////////////////////

class PacketPrepareForUserLogin : public PacketLogin
{
public:
   PacketPrepareForUserLogin() : PacketLogin( PacketType_Login, PacketLogin::LoginType_PrepareForUserLogin ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   lastLoginTime;
   U32      connectionId;
   string   email;
   bool     active;
   U32      userId;
};

///////////////////////////////////////////////////////////////

class PacketPrepareForUserLogout : public BasePacket
{
public:
   PacketPrepareForUserLogout() : BasePacket( PacketType_Login, PacketLogin::LoginType_PrepareForUserLogout ), connectionId( 0 ), wasDisconnectedByError( false ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string uuid;
   U32   connectionId;
   bool  wasDisconnectedByError;
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

      ErrorType_Contact_Asset_BadLoginKey,
      
      ErrorType_Cheat_BadPermissions,
      ErrorType_Cheat_UnrecognizedCommand,
      ErrorType_Cheat_UnknownError,
      ErrorType_Cheat_ProductUnknown,
      ErrorType_Login_CannotAddCurrentProductToUser,

   };

   enum StatusSubtype
   {
      StatusSubtype_ProductAdded,
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