bin_PROGRAMS = vcmiclient
BUILT_SOURCES = StdInc.h.gch
StdInc.h.gch: StdInc.h
	$(CXXCOMPILE) -c $<

vcmiclient_LDADD = $(top_builddir)/lib/libvcmi.la @SDL_LIBS@ @FFMPEG_LIBS@ @BOOST_PROGRAM_OPTIONS_LIB@ @BOOST_SYSTEM_LIB@ @BOOST_FILESYSTEM_LIB@ @BOOST_THREAD_LIB@ @BOOST_IOSTREAMS_LIB@ -lz -lrt
vcmiclient_CXXFLAGS = @SDL_CFLAGS@ @FFMPEG_CXXFLAGS@
vcmiclient_LDFLAGS = -L$(top_builddir)/lib
vcmiclient_SOURCES =  \
	../CCallback.cpp \
	./BattleInterface/CBattleAnimations.cpp \
	./BattleInterface/CBattleInterface.cpp \
	./BattleInterface/CBattleInterfaceClasses.cpp \
	./BattleInterface/CCreatureAnimation.cpp \
	./UIFramework/AnimationClasses.cpp \
	./UIFramework/CCursorHandler.cpp \
	./UIFramework/CGuiHandler.cpp \
	./UIFramework/CIntObject.cpp \
	./UIFramework/CIntObjectClasses.cpp \
	./UIFramework/Geometries.cpp \
	./UIFramework/ImageClasses.cpp \
	./UIFramework/SDL_Extensions.cpp \
	CAdvmapInterface.cpp \
	CAdvmapInterface.h \
	CAnimation.cpp \
	CAnimation.h \
	CBitmapHandler.cpp \
	CBitmapHandler.h \
	CCastleInterface.cpp \
	CCastleInterface.h \
	CConfigHandler.cpp \
	CConfigHandler.h \
	CCreatureWindow.cpp \
	CCreatureWindow.h \
	CDefHandler.cpp \
	CDefHandler.h \
	CGameInfo.cpp \
	CGameInfo.h \
	CHeroWindow.cpp \
	CHeroWindow.h \
	CKingdomInterface.cpp \
	CKingdomInterface.h \
	Client.cpp \
	Client.h \
	CMessage.cpp \
	CMessage.h \
	CMT.cpp \
	CMusicBase.h \
	CMusicHandler.cpp \
	CMusicHandler.h \
	CPlayerInterface.cpp \
	CPlayerInterface.h \
	CPreGame.cpp \
	CPreGame.h \
	CResourceHandler.cpp \
	CSndHandler.cpp \
	CSndHandler.h \
	CSoundBase.h \
	CSpellWindow.cpp \
	CSpellWindow.h \
	CVideoHandler.cpp \
	CVideoHandler.h \
	FontBase.h \
	FunctionList.h \
	Graphics.cpp \
	Graphics.h \
	GUIClasses.cpp \
	GUIClasses.h \
	mapHandler.cpp \
	mapHandler.h \
	NetPacksClient.cpp
