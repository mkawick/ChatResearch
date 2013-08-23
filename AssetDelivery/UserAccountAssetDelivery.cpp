#include "UserAccountAssetDelivery.h"

UserAccountAssetDelivery::UserAccountAssetDelivery( const UserTicket& ticket ) : m_userTicket( ticket ), m_status( Status_initial_login ), m_readyForCleanup( false ), m_assetManager( NULL )
{
}

UserAccountAssetDelivery::~UserAccountAssetDelivery()
{
}

void  UserAccountAssetDelivery::Update()
{
   if( m_assetManager == NULL )
      return;
}

void     UserAccountAssetDelivery::UserLoggedOut()
{
   m_status = Status_awaiting_cleanup;
   m_readyForCleanup = true;
   time( &m_logoutTime );
}
//------------------------------------------------------------------------------------------------

bool     UserAccountAssetDelivery::HandleRequestFromClient( const PacketAsset* packet )
{
  /* switch( packet->packetSubType )
   {
   case PacketContact::ContactType_GetListOfContacts:
      return GetListOfContacts();

   case PacketContact::ContactType_GetListOfInvitations:
      return GetListOfInvitationsReceived();

   case PacketContact::ContactType_GetListOfInvitationsSent:
      return GetListOfInvitationsSent();

   case PacketContact::ContactType_InviteContact:
      return InviteUser( static_cast< const PacketContact_InviteContact* >( packet ) );
      
   case PacketContact::ContactType_AcceptInvite:
      return AcceptInvitation( static_cast< const PacketContact_AcceptInvite* >( packet ) );

   case PacketContact::ContactType_DeclineInvitation:
      return DeclineInvitation( static_cast< const PacketContact_DeclineInvitation* >( packet ) );
   }*/

   return false;
}

//------------------------------------------------------------------------------------------------