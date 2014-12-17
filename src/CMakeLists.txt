set(sdr_qrss_SOURCES main.cc options.cc
    qrss.cc receiver.cc mainwindow.cc)
set(sdr_qrss_MOC_HEADERS
    qrss.hh receiver.hh mainwindow.hh)
qt5_wrap_cpp(sdr_qrss_MOC_SOURCES ${sdr_qrss_MOC_HEADERS})

set(sdr_qrss_HEADERS ${sdr_qrss_MOC_HEADERS} options.hh)

add_executable(sdr-qrss ${sdr_qrss_SOURCES} ${sdr_qrss_MOC_SOURCES})

target_link_libraries(sdr-qrss
 ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${LIBS})

INSTALL(TARGETS sdr-qrss DESTINATION bin)
