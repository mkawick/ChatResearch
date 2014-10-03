bin_PROGRAMS = login_serverd

login_serverddir = $(datadir)/@PACKAGE@

login_serverd_SOURCES = ConnectionToUser.cpp \
CreateAccountResultsAggregator.cpp \
DiplodocusLogin.cpp \
FruitadensLogin.cpp \
KhaanLogin.cpp \
Login.cpp \
../NetworkCommon/NetworkUtils.cpp \
../NetworkCommon/ServerConstants.cpp \
../NetworkCommon/ServerType.cpp \
../NetworkCommon/ChainedArchitecture/Thread.cpp \
../NetworkCommon/Daemon/Daemonizer.cpp \
../NetworkCommon/Database/Deltadromeus.cpp \
../NetworkCommon/Database/StringLookup.cpp \
../NetworkCommon/Logging/server_log.cpp \
../NetworkCommon/NetworkIn/Diplodocus.cpp \
../NetworkCommon/NetworkIn/DiplodocusServerToServer.cpp \
../NetworkCommon/NetworkIn/Khaan.cpp \
../NetworkCommon/NetworkIn/KhaanServerToServer.cpp \
../NetworkCommon/NetworkOut/Fruitadens.cpp \
../NetworkCommon/NetworkOut/FruitadensServerToServer.cpp \
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
../NetworkCommon/Utils/OsLevelUtils.cpp \
../NetworkCommon/Utils/TableWrapper.cpp \
../NetworkCommon/Utils/Utils.cpp 

login_serverd_CXXFLAGS = -std=c++0x

login_serverd_LDFLAGS = -L

login_serverd_LDADD = /usr/local/lib/libevent.a \
/usr/local/lib/libevent_core.a \
/usr/local/lib/libevent_pthreads.a \
/usr/local/lib/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/login_serverd
	sudo chmod u+s $(bindir)/login_serverd


