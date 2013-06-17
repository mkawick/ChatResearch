bin_PROGRAMS = login_serverd

login_serverddir = $(datadir)/@PACKAGE@

login_serverd_SOURCES = DiplodocusLogin.cpp \
email.cpp \
FruitadensLogin.cpp \
KhaanLogin.cpp \
Login.cpp \
../NetworkCommon/NetworkUtils.cpp \
../NetworkCommon/ChainedArchitecture/Thread.cpp \
../NetworkCommon/Daemon/Daemonizer.cpp \
../NetworkCommon/Database/Deltadromeus.cpp \
../NetworkCommon/NetworkIn/Diplodocus.cpp \
../NetworkCommon/NetworkIn/Khaan.cpp \
../NetworkCommon/NetworkIn/KhaanServerToServer.cpp \
../NetworkCommon/NetworkOut/Fruitadens.cpp \
../NetworkCommon/Packets/BasePacket.cpp \
../NetworkCommon/Packets/ChatPacket.cpp \
../NetworkCommon/Packets/DbPacket.cpp \
../NetworkCommon/Packets/GamePacket.cpp \
../NetworkCommon/Packets/ServerToServerPacket.cpp \
../NetworkCommon/Packets/PacketFactory.cpp \
../NetworkCommon/Utils/TableWrapper.cpp \
../NetworkCommon/Utils/CommandLineParser.cpp \
../NetworkCommon/Utils/Utils.cpp \
../NetworkCommon/Logging/server_log.cpp

login_serverd_LDFLAGS = -L

login_serverd_LDADD = ../../libevent/linux-libs/libevent.a \
../../libevent/linux-libs/libevent_pthreads.a \
../../libevent/linux-libs/libevent_core.a \
../../libevent/linux-libs/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/login_serverd
	sudo chmod u+s $(bindir)/login_serverd


