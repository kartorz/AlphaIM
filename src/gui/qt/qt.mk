
gui/qt/moc_aimwin.cpp: gui/qt/aimwin.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/aimwin.h       -o gui/qt/moc_aimwin.cpp

gui/qt/moc_GuiMessager.cpp: gui/qt/GuiMessager.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/GuiMessager.h  -o gui/qt/moc_GuiMessager.cpp

gui/qt/moc_icwin.cpp: gui/qt/icwin.h
	$(MOC) $(DEFINES) $(INCLUDES) gui/qt/icwin.h       -o gui/qt/moc_icwin.cpp

gui/qt/ui_aimwin.h: gui/qt/aimwin.ui
	$(UIC) gui/qt/aimwin.ui -o gui/qt/ui_aimwin.h

gui/qt/ui_icwin.h: gui/qt/icwin.ui
	$(UIC) gui/qt/icwin.ui -o gui/qt/ui_icwin.h

gui/qt/qrc_default.cpp:  gui/qt/default.qrc
	$(RCC) --no-compress -name default  gui/qt/default.qrc -o  gui/qt/qrc_default.cpp

