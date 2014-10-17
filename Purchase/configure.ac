AC_INIT([purchase_serverd], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
   Makefile
])

CXXFLAGS="$CXXFLAGS -DNDEBUG -Wall -I/usr/local/include -I/usr/include/boost -I/usr/include/mysql -DBIG_JOINS=1  -fno-strict-aliasing   -DUNIV_LINUX -DUNIV_LINUX "

LDFLAGS="-rdynamic -L/usr/lib/mysql -lmysqlclient -lssl -lcrypto -lz"

AC_OUTPUT
