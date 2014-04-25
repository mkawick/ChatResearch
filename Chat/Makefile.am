bin_PROGRAMS = chat_serverd

chat_serverddir = $(datadir)/@PACKAGE@

chat_serverd_SOURCES = Chat.cpp \
ChatChannel.cpp \
ChatChannelDbJob.cpp \
ChatChannelManager.cpp \
ChatUser.cpp \
DiplodocusChat.cpp \
KhaanChat.cpp \
UserInChatChannel.cpp \
UsersChatChannelList.cpp \
../NetworkCommon/NetworkUtils.cpp \
../NetworkCommon/ServerConstants.cpp \
../NetworkCommon/ServerType.cpp \
../NetworkCommon/ChainedArchitecture/Thread.cpp \
../NetworkCommon/Daemon/Daemonizer.cpp \
../NetworkCommon/Database/Deltadromeus.cpp \
../NetworkCommon/NetworkIn/Diplodocus.cpp \
../NetworkCommon/NetworkIn/DiplodocusServerToServer.cpp \
../NetworkCommon/NetworkIn/Khaan.cpp \
../NetworkCommon/NetworkIn/KhaanServerToServer.cpp \
../NetworkCommon/NetworkOut/Fruitadens.cpp \
../NetworkCommon/Packets/AssetPacket.cpp \
../NetworkCommon/Packets/BasePacket.cpp \
../NetworkCommon/Packets/ChatPacket.cpp \
../NetworkCommon/Packets/CheatPacket.cpp \
../NetworkCommon/Packets/CommonTypes.cpp \
../NetworkCommon/Packets/ContactPacket.cpp \
../NetworkCommon/Packets/DbPacket.cpp \
../NetworkCommon/Packets/GamePacket.cpp \
../NetworkCommon/Packets/LoginPacket.cpp \
../NetworkCommon/Packets/NotificationPacket.cpp \
../NetworkCommon/Packets/PacketFactory.cpp \
../NetworkCommon/Packets/PurchasePacket.cpp \
../NetworkCommon/Packets/Serialize.cpp \
../NetworkCommon/Packets/ServerToServerPacket.cpp \
../NetworkCommon/Packets/StatPacket.cpp \
../NetworkCommon/Packets/TournamentPacket.cpp \
../NetworkCommon/Stat/StatTrackingConnections.cpp \
../NetworkCommon/Utils/CommandLineParser.cpp \
../NetworkCommon/Utils/MemoryLeakTracker.cpp \
../NetworkCommon/Utils/OsLevelUtils.cpp \
../NetworkCommon/Utils/TableWrapper.cpp \
../NetworkCommon/Utils/Utils.cpp \
../NetworkCommon/Logging/server_log.cpp

chat_serverd_CXXFLAGS = -std=c++0x

chat_serverd_LDFLAGS = -L

chat_serverd_LDADD = ../../libevent/linux-libs/libevent.a \
../../libevent/linux-libs/libevent_pthreads.a \
../../libevent/linux-libs/libevent_core.a \
../../libevent/linux-libs/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/chat_serverd
	sudo chmod u+s $(bindir)/chat_serverd


