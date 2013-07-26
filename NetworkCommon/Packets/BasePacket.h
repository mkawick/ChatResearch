// BasePacket.h

#pragma once

#include "../ServerType.h"
#include "../DataTypes.h"
#include "../ServerConstants.h"
#include "../Serialize.h"
#include <string>
#include <vector>
#include <list>
using namespace std;

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
   PacketType_ErrorReport
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

class DynamicDataBucket
{
public:
   typedef vector< string >   DataRow;
   typedef list< DataRow >  DataSet;

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   void  operator = ( const list< list< string > >& copyData );
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

protected:
   vector< type >    m_data;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class BasePacket
{
protected:
   //enum SubType {};
public:
   BasePacket( int packet_type = PacketType_Base, int packet_sub_type = 0 ) :
      packetType( packet_type ),
      packetSubType( packet_sub_type ),
      versionNumber( 0 ),
      gameInstanceId( 0 ){}
   ~BasePacket()
   {
      gameInstanceId = 0;// for a place to set breakpoints.
   }

   virtual bool  SerializeIn( const U8* data, int& bufferOffset );
   virtual bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U8       packetType;
   U8       packetSubType;
   U8       versionNumber;
   U8       gameProductId;
   U32      gameInstanceId;
};

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
      LoginType_CreateAccountResponse
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

   bool  wasLoginSuccessful;
   U32   connectionId;  // used for a user ID.
};


///////////////////////////////////////////////////////////////

class PacketLoginToGateway : public PacketLogin
{
public:
   PacketLoginToGateway(): PacketLogin( PacketType_Login, PacketLogin::LoginType_InformGatewayOfLoginStatus ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   bool  wasLoginSuccessful;
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
   ChannelInfo( const string& name, bool active) : channelName( name ), isActive( active ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   channelName;
   bool     isActive;
};

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
      InfoType_GroupsList
   };
public:
   PacketUserInfo( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_None ) : BasePacket( packet_type, packet_sub_type ){  }
};

///////////////////////////////////////////////////////////////

class PacketFriendsListRequest : public PacketUserInfo
{
public:
   PacketFriendsListRequest( int packet_type = PacketType_UserInfo, int packet_sub_type = InfoType_FriendsListRequest ) : PacketUserInfo( packet_type, packet_sub_type ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return BasePacket::SerializeIn( data, bufferOffset ); }
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return BasePacket::SerializeOut( data, bufferOffset ); }
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

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return BasePacket::SerializeIn( data, bufferOffset ); }
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return BasePacket::SerializeOut( data, bufferOffset ); }
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
   PacketGatewayWrapper( int packet_type = PacketType_GatewayWrapper, int packet_sub_type = 0  ): BasePacket( packet_type, packet_sub_type ), connectionId( -1 ), pPacket( NULL ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32         connectionId;
   BasePacket* pPacket;
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
      ErrorType_UserBadLogin,
      ErrorType_UserDoesNotExist,
      ErrorType_UserBlocked,
      ErrorType_UserLoggedOut,

      ErrorType_ChatNotCurrentlyAvailable,// reported at the gateway
      ErrorType_BadChatChannel,
      ErrorType_NoChatChannel,
      ErrorType_NotAMemberOfThatChatChannel,
      ErrorType_YouAreTheOnlyPersonInThatChatChannel,
      ErrorType_CannotAddUserToChannel_AlreadyExists,
      ErrorType_NoChatHistoryExistsOnSelectedChannel,
      ErrorType_NoChatHistoryExistsForThisUser,

      ErrorType_CreateFailed_BadPassword,
      ErrorType_CreateFailed_DisallowedUsername,
      ErrorType_CreateFailed_DuplicateUsername,
      ErrorType_CreateFailed_DuplicateEmail,
      ErrorType_CreateFailed_UserCreateAccountPending,
      ErrorType_CreateAccount_Success,
      ErrorType_CreateAccount_AccountUpdated,

      ErrorType_Contact_Invitation_success,
      ErrorType_Contact_Invitation_ProblemFindingUser,
      ErrorType_Contact_Invitation_InviteeAlreadyInvited,
      ErrorType_Contact_Invitation_Accepted,
   };
public:
   PacketErrorReport( int packet_sub_type = ErrorType_Generic ): BasePacket( PacketType_ErrorReport, packet_sub_type ) {}
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

#include "BasePacket.inl"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  PackageForServerIdentification( const string& serverName, U32 serverId, U8 gameProductId, bool isGameServer, bool isController, bool requiresWrapper, bool isGateway, BasePacket** packet );