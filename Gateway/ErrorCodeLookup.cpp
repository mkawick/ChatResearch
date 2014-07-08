// ErrorCodeLookup.cpp

#include "ErrorCodeLookup.h"
#include "../NetworkCommon/Packets/BasePacket.h"

string  ErrorCodeLookup::GetString( int id )
{
   switch( id )
   {
      case PacketErrorReport::ErrorType_Generic:                                       return string( "generic" );
      case PacketErrorReport::ErrorType_Status:                                        return string( "status" );
      case PacketErrorReport::ErrorType_Login:                                         return string( "login" );
      case PacketErrorReport::ErrorType_UserBadLogin:                                  return string( "bad login" );
      case PacketErrorReport::ErrorType_UserDoesNotExist:                              return string( "user does not exist" );
      case PacketErrorReport::ErrorType_UserBlocked:                                   return string( "user blocked" );
      case PacketErrorReport::ErrorType_UserLoggedOut:                                 return string( "user logged out" );
      case PacketErrorReport::ErrorType_UserNotFinishedWithLogin:                      return string( "User Not Finished With Login" );
      case PacketErrorReport::ErrorType_Login_unknown1:                                return string( "login unknown1" );
      case PacketErrorReport::ErrorType_Login_unknown2:                                return string( "login unknown2" );
      case PacketErrorReport::ErrorType_ChatNotCurrentlyAvailable:                     return string( "chat is not avail" );
      case PacketErrorReport::ErrorType_BadChatChannel:                                return string( "Bad Chat Channel" );
      case PacketErrorReport::ErrorType_NoChatChannel:                                 return string( "No Chat Channel" );
      case PacketErrorReport::ErrorType_UserNotOnline:                                 return string( "User Not Online" );
      case PacketErrorReport::ErrorType_NotAMemberOfThatChatChannel:                   return string( "Not A Member O fThat Chat Channel" );
      case PacketErrorReport::ErrorType_YouAreTheOnlyPersonInThatChatChannel:          return string( "You Are The Only Person In That Chat Channel" );
      case PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists:          return string( "Cannot Add User To Channel-Already Exists" );
      case PacketErrorReport::ErrorType_NoChatHistoryExistsOnSelectedChannel:          return string( "No Chat History Exists On Selected Channel" );
      case PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser:                return string( "No Chat History Exists For This User" );
      case PacketErrorReport::ErrorType_ChatChannelCannotBeCreated:                    return string( "Chat Channel Cannot Be Created" );
      case PacketErrorReport::ErrorType_ChatChannelCannotBeDeleted:                    return string( "Chat Channel Cannot Be Deleted" );
      case PacketErrorReport::ErrorType_UserUnknown:                                   return string( "User Unknown" );
      case PacketErrorReport::ErrorType_UserIsBlockingFriendInvites:                   return string( "User Is Blocking Friend Invites" );
      case PacketErrorReport::ErrorType_CannotAddUserToChannel:                        return string( "CannotAddUserToChannel" );
      case PacketErrorReport::ErrorType_ChatChannel_UserNotAcceptingInvites:           return string( "User is not accepting chat channel invites" );
      case PacketErrorReport::ErrorType_Chat_unknown2:                                 return string( "chat unknown 2" );
      case PacketErrorReport::ErrorType_CreateFailed_BadPassword:                      return string( "CreateFailed_BadPassword" );
      case PacketErrorReport::ErrorType_CreateFailed_DisallowedUsername:               return string( "CreateFailed_DisallowedUsername" );
      case PacketErrorReport::ErrorType_CreateFailed_DuplicateUsername:                return string( "CreateFailed_DuplicateUsername" );
      case PacketErrorReport::ErrorType_CreateFailed_DuplicateEmail:                   return string( "CreateFailed_DuplicateEmail" );
      case PacketErrorReport::ErrorType_CreateFailed_UserCreateAccountPending:         return string( "CreateFailed_UserCreateAccountPending" );
      case PacketErrorReport::ErrorType_CreateAccount_Success:                         return string( "CreateAccount_Success" );
      case PacketErrorReport::ErrorType_CreateAccount_AccountUpdated:                  return string( "CreateAccount_AccountUpdated" );
      case PacketErrorReport::ErrorType_CreateAccount_unknown1:                        return string( "CreateAccount_unknown1" );
      case PacketErrorReport::ErrorType_CreateAccount_unknown2:                        return string( "CreateAccount_unknown2" );
      case PacketErrorReport::ErrorType_Contact_Invitation_success:                    return string( "Contact_Invitation_success" );
      case PacketErrorReport::ErrorType_Contact_Invitation_ProblemFindingUser:         return string( "Contact_Invitation_ProblemFindingUser" );
      case PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyInvited:      return string( "Contact_Invitation_InviteeAlreadyInvited" );
      case PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyFriend:       return string( "Contact_Invitation_InviteeAlreadyFriend" );
      case PacketErrorReport::ErrorType_Contact_Invitation_InvalidUser:                return string( "Contact_Invitation_InvalidUser" );
      case PacketErrorReport::ErrorType_Contact_Invitation_CannotInviteSelf:           return string( "Contact_Invitation_CannotInviteSelf" );
      case PacketErrorReport::ErrorType_Contact_Invitation_Accepted:                   return string( "Contact_Invitation_Accepted" );
      case PacketErrorReport::ErrorType_Contact_Invitation_BadInvitation:              return string( "Contact_Invitation_BadInvitation" );
      case PacketErrorReport::ErrorType_Contact_BadLoginKey:                           return string( "Contact_BadLoginKey" );
      case PacketErrorReport::ErrorType_Contact_BadSearchString:                       return string( "Contact_BadSearchString" );
      case PacketErrorReport::ErrorType_Contact_NoSearchResult:                        return string( "Contact_NoSearchResult" );
      case PacketErrorReport::ErrorType_Contact_SearchRequestHasTooMany:               return string( "Contact_SearchRequestHasTooMany" );
      case PacketErrorReport::ErrorType_Contact_NotAUserContact:                       return string( "Contact_NotAUserContact" );
      case PacketErrorReport::ErrorType_Contact_Invitation_InviteeSentYouInvitation:   return string( "Contact_Invitation_InviteeSentYouInvitation" ); 
      case PacketErrorReport::ErrorType_Contact_unknown1:                              return string( "Contact_unknown1" );
      case PacketErrorReport::ErrorType_Contact_unknown2:                              return string( "Contact_unknown2" );
      case PacketErrorReport::ErrorType_Asset_BadLoginKey:                             return string( "Asset BadLoginKey" );
      case PacketErrorReport::ErrorType_Asset_UnknownAsset:                            return string( "Asset UnknownAsset" );
      case PacketErrorReport::ErrorType_Asset_UserDisconnected:                        return string( "Asset UserDisconnected" );
      case PacketErrorReport::ErrorType_Asset_UnknownAssetCategory:                    return string( "Asset UnknownAssetCategory" );
      case PacketErrorReport::ErrorType_Asset_NoCategoriesAvailable:                   return string( "Asset NoCategoriesAvailable" );
      case PacketErrorReport::ErrorType_Asset_unknown1:                                return string( "Asset_unknown1" );
      case PacketErrorReport::ErrorType_Asset_unknown2:                                return string( "Asset_unknown2" );
      case PacketErrorReport::ErrorType_Cheat_BadPermissions:                          return string( "Cheat_BadPermissions" );
      case PacketErrorReport::ErrorType_Cheat_BadUserLookup:                           return string( "Cheat_BadUserLookup" );
      case PacketErrorReport::ErrorType_Cheat_BadUserLookup_TryLoadingUserFirst:       return string( "Cheat_BadUserLookup_TryLoadingUserFirst" );
      case PacketErrorReport::ErrorType_Cheat_UnrecognizedCommand:                     return string( "Cheat_UnrecognizedCommand" );
      case PacketErrorReport::ErrorType_Cheat_UnknownError:                            return string( "Cheat_UnknownError" );
      case PacketErrorReport::ErrorType_Cheat_ProductUnknown:                          return string( "Cheat_ProductUnknown" );
      case PacketErrorReport::ErrorType_Login_CannotAddCurrentProductToUser:           return string( "Login_CannotAddCurrentProductToUse" );
      case PacketErrorReport::ErrorType_Login_ProfileIsAlreadyBeingUpdated:            return string( "Login_ProfileIsAlreadyBeingUpdated" );
      case PacketErrorReport::ErrorType_Cheat_unknown1:                                return string( "Cheat_unknown1" );
      case PacketErrorReport::ErrorType_Cheat_unknown2:                                return string( "Cheat_unknown2" );
      case PacketErrorReport::ErrorType_Purchase_BadPurchaseId:                        return string( "Purchase BadPurchaseId" );
      case PacketErrorReport::ErrorType_Purchase_StoreBusy:                            return string( "Purchase StoreBusy" );
      case PacketErrorReport::ErrorType_Purchase_UserDoesNotHaveEnoughToTrade:         return string( "Purchase UserDoesNotHaveEnoughToTrade" );
      case PacketErrorReport::ErrorType_Purchase_UserInventoryIsFull:                  return string( "Purchase UserInventoryIsFull" );
      case PacketErrorReport::ErrorType_Purchase_ItemNotAvailable:                     return string( "Purchase ItemNotAvailable" );
      case PacketErrorReport::ErrorType_Purchase_TimePeriodHasNotBegunYet:             return string( "Purchase TimePeriodHasNotBegunYet" );
      case PacketErrorReport::ErrorType_Purchase_TimePeriodHasExpired:                 return string( "Purchase TimePeriodHasExpired" );
      case PacketErrorReport::ErrorType_Purchase_UserCannotPurchaseAnyMoreOfThese:     return string( "Purchase UserCannotPurchaseAnyMoreOfThese" );
      case PacketErrorReport::ErrorType_Purchase_AllPurchasingIsClosedRightNow:        return string( "Purchase AllPurchasingIsClosedRightNow" );
      case PacketErrorReport::ErrorType_Purchase_Success:                              return string( "Purchase Success" );
      case PacketErrorReport::ErrorType_Purchase_ProductUnknown:                       return string( "Purchase: product is unknown" );
      case PacketErrorReport::ErrorType_Purchase_NoTradeItemsSpecified:                return string( "Purchase: no items are specified for payment" );
      case PacketErrorReport::ErrorType_Purchase_DuplicateItemsForPayment:             return string( "Purchase: duplicate items in purchase request" );
      case PacketErrorReport::ErrorType_TournamentPurchase_Result_PurchasePending:     return string( "TournamentPurchase_Result_PurchasePending" );
      case PacketErrorReport::ErrorType_TournamentPurchase_Result_TooManyPlayers:      return string( "TournamentPurchase_Result_TooManyPlayers" );
      case PacketErrorReport::ErrorType_TournamentPurchase_Result_RequirementsNotMet:  return string( "TournamentPurchase_Result_RequirementsNotMet" );
      case PacketErrorReport::ErrorType_TournamentPurchase_Result_TournamentClosed:    return string( "TournamentPurchase_Result_TournamentClosed" );
      case PacketErrorReport::ErrorType_TournamentPurchase_Result_UnknownUuid:         return string( "TournamentPurchase_Result_UnknownUuid" );
      case PacketErrorReport::ErrorType_TournamentPurchase_unknown1:                   return string( "TournamentPurchase_unknown1" );
      case PacketErrorReport::ErrorType_TournamentPurchase_unknown2:                   return string( "TournamentPurchase_unknown2" );
      case PacketErrorReport::ErrorType_Notification_DeviceAlreadyRegistered:          return string( "Notification_DeviceAlreadyRegistered" );
      case PacketErrorReport::ErrorType_Notification_TooManyDevices:                   return string( "Notification_TooManyDevices" );
      case PacketErrorReport::ErrorType_Notification_DeviceIdIncorrect:                return string( "Notification_DeviceIdIncorrect" );
      case PacketErrorReport::ErrorType_Notification_NoDevicesListed:                  return string( "Notification_NoDevicesListed" );
      case PacketErrorReport::ErrorType_Notification_NoDevicesEnabledForThisGame:      return string( "Notification_NoDevicesEnabledForThisGame" );
      case PacketErrorReport::ErrorType_Notification_CannotInsertNewDevice:            return string( "Notification_CannotInsertNewDevice" );
      case PacketErrorReport::ErrorType_Notification_unknown1:                         return string( "Notification_unknown1" );
      case PacketErrorReport::ErrorType_Notification_unknown2:                         return string( "Notification_unknown2" );
      case PacketErrorReport::ErrorType_Invitation_ExistingInvitationWithThatUser:     return string( "An Invitation already exists for the group with these two users" );
      case PacketErrorReport::ErrorType_Invitation_DoesNotExist:                       return string( "Invalid invitation" );
      case PacketErrorReport::ErrorType_Invitation_UserIsBlockingInvites:              return string( "User is blocking invitations" );
      case PacketErrorReport::ErrorType_Invitation_BadServerSetup:                     return string( "Server is not configured properly for invites of this kind" );

      case PacketErrorReport::ErrorType_IncompleteFeature:                             return string( "This feature is incompete" );
   };
   return string();
}