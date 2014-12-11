bin_PROGRAMS = user_stats_serverd

user_stats_serverddir = $(datadir)/@PACKAGE@

user_stats_serverd_SOURCES = server_rating_glicko2.cpp \
server_stats_agricola.cpp \
server_stats_ascension.cpp \
server_stats_fluxx.cpp \
server_stats_foodfight.cpp \
server_stats_nightfall.cpp \
server_stats_pennyarcade.cpp \
server_stats_plugins.cpp \
server_stats_summonwar.cpp \
server_stats_tantocuore.cpp \
server_stats_waterdeep.cpp \
UserStats.cpp \
UserStatsMainThread.cpp \
../NetworkCommon/NetworkUtils.cpp \
../NetworkCommon/ServerConstants.cpp \
../NetworkCommon/ServerType.cpp \
../NetworkCommon/ChainedArchitecture/Thread.cpp \
../NetworkCommon/Daemon/Daemonizer.cpp \
../NetworkCommon/Database/Deltadromeus.cpp \
../NetworkCommon/Database/StringLookup.cpp \
../NetworkCommon/Logging/server_log.cpp \
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

user_stats_serverd_CXXFLAGS = -std=c++0x

user_stats_serverd_LDFLAGS = -L

user_stats_serverd_LDADD = /usr/local/lib/libevent.a \
/usr/local/lib/libevent_core.a \
/usr/local/lib/libevent_pthreads.a \
/usr/local/lib/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/user_stats_serverd
	sudo chmod u+s $(bindir)/user_stats_serverd


