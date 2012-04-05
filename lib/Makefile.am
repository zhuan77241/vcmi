BUILT_SOURCES = StdInc.h.gch
StdInc.h.gch: StdInc.h
	$(CXXCOMPILE) -DVCMI_DLL -fPIC -c $<

pkglib_LTLIBRARIES = libvcmi.la
libvcmi_la_LIBADD = @BOOST_SYSTEM_LIB@ @BOOST_FILESYSTEM_LIB@ @BOOST_THREAD_LIB@ @BOOST_IOSTREAMS_LIB@ -lz -lpthread -ldl
libvcmi_la_CXXFLAGS = @SDL_CFLAGS@ -DVCMI_DLL
libvcmi_la_SOURCES = \
	BattleAction.cpp \
	BattleHex.cpp \
	BattleState.cpp \
	CArtHandler.cpp \
	CBuildingHandler.cpp \
	CCampaignHandler.cpp \
	CConsoleHandler.cpp \
	CCreatureHandler.cpp \
	CCreatureSet.cpp \
	CDefObjInfoHandler.cpp \
	CFileSystemHandler.cpp \
	CGameInterface.cpp \
	CGameState.cpp \
	CGeneralTextHandler.cpp \
	CHeroHandler.cpp \
	CLodHandler.cpp \
	CLogger.cpp \
	CMapInfo.cpp \
	CObjectHandler.cpp \
	Connection.cpp \
	CSpellHandler.cpp \
	CTownHandler.cpp \
	CThreadHelper.cpp \
	HeroBonus.cpp \
	IGameCallback.cpp \
	JsonNode.cpp \
	map.cpp \
	NetPacksLib.cpp \
	ResourceSet.cpp \
	VCMI_Lib.cpp
