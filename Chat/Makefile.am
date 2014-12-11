bin_PROGRAMS = chat_serverd

chat_serverddir = $(datadir)/@PACKAGE@

chat_serverd_SOURCES = Chat.cpp \
ChatRoom.cpp \
ChatRoomDbJob.cpp \
ChatRoomManager.cpp \
ChatUser.cpp \
DiplodocusChat.cpp \
KhaanChat.cpp \
UserInChatRoom.cpp \
UsersChatRoomList.cpp \
../NetworkCommon/NetworkUtils.cpp \
../NetworkCommon/ServerConstants.cpp \
../NetworkCommon/ServerType.cpp \
../NetworkCommon/ChainedArchitecture/Thread.cpp \
../NetworkCommon/Daemon/Daemonizer.cpp \
../NetworkCommon/Database/Deltadromeus.cpp \
../NetworkCommon/Logging/server_log.cpp \
../NetworkCommon/Invitations/InvitationManager.cpp \
../NetworkCommon/NetworkIn/Diplodocus.cpp \
../NetworkCommon/NetworkIn/DiplodocusTools.cpp \
../NetworkCommon/NetworkIn/DiplodocusServerToServer.cpp \
../NetworkCommon/NetworkIn/Khaan.cpp \
../NetworkCommon/NetworkIn/KhaanServerToServer.cpp \
../NetworkCommon/NetworkOut/Fruitadens.cpp \
../NetworkCommon/Packets/AnalyticsPacket.cpp \
../NetworkCommon/Packets/AssetPacket.cpp \
../NetworkCommon/Packets/BasePacket.cpp \
../NetworkCommon/Packets/ChatPacket.cpp \
../NetworkCommon/Packets/CheatPacket.cpp \
../NetworkCommon/Packets/CommonTypes.cpp \
../NetworkCommon/Packets/ContactPacket.cpp \
../NetworkCommon/Packets/DbPacket.cpp \
../NetworkCommon/Packets/GamePacket.cpp \
../NetworkCommon/Packets/InvitationPacket.cpp \
../NetworkCommon/Packets/LoginPacket.cpp \
../NetworkCommon/Packets/NotificationPacket.cpp \
../NetworkCommon/Packets/PacketFactory.cpp \
../NetworkCommon/Packets/PurchasePacket.cpp \
../NetworkCommon/Packets/Serialize.cpp \
../NetworkCommon/Packets/ServerToServerPacket.cpp \
../NetworkCommon/Packets/TournamentPacket.cpp \
../NetworkCommon/Packets/UserStatsPacket.cpp \
../NetworkCommon/Stat/StatTrackingConnections.cpp \
../NetworkCommon/Utils/CommandLineParser.cpp \
../NetworkCommon/Utils/KeepAliveSignaller.cpp \
../NetworkCommon/Utils/MemoryLeakTracker.cpp \
../NetworkCommon/Utils/OsLevelUtils.cpp \
../NetworkCommon/Utils/TableWrapper.cpp \
../NetworkCommon/Utils/StringUtils.cpp \
../NetworkCommon/Utils/Utils.cpp 

chat_serverd_CXXFLAGS = -std=c++0x

chat_serverd_LDFLAGS = -L

chat_serverd_LDADD = /usr/local/lib/libevent.a \
/usr/local/lib/libevent_core.a \
/usr/local/lib/libevent_pthreads.a \
/usr/local/lib/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/chat_serverd
	sudo chmod u+s $(bindir)/chat_serverd


