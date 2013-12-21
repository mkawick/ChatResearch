lib_LTLIBRARIES = mBer_packet_library.la

mBer_packet_library_includedir=$(includedir)/something-1.0/something
mBer_packet_library_include_HEADERS = something/foo.h something/bar.h

mBer_packet_library_SOURCES = \
../NetworkCommon/Packets/AssetPacket.cpp \
../NetworkCommon/Packets/BasePacket.cpp \
../NetworkCommon/Packets/ChatPacket.cpp \
../NetworkCommon/Packets/CheatPacket.cpp \
../NetworkCommon/Packets/ContactPacket.cpp \
../NetworkCommon/Packets/DataBucket.cpp \
../NetworkCommon/Packets/DbPacket.cpp \
../NetworkCommon/Packets/GamePacket.cpp \
../NetworkCommon/Packets/LoginPacket.cpp \
../NetworkCommon/Packets/ServerToServerPacket.cpp \
../NetworkCommon/Packets/PacketFactory.cpp \
../NetworkCommon/Packets/PurchasePacket.cpp \
../NetworkCommon/Packets/TournamentPacket.cpp 

mBer_packet_library_LDFLAGS = -version-info $(0.38)

mBer_packet_library_LDADD = ../../libevent/linux-libs/libevent.a \
../../libevent/linux-libs/libevent_pthreads.a \
../../libevent/linux-libs/libevent_core.a \
../../libevent/linux-libs/libevent_extra.a \
-lrt

install-exec-hook:
	chown root $(bindir)/mBer_packet_library
	sudo chmod u+s $(bindir)/mBer_packet_library


