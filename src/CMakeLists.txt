set(sdr_qrss_SOURCES main.cc
    qrss.cc options.cc)
set(sdr_qrss_MOC_HEADERS
    qrss.hh)
qt5_wrap_cpp(sdr_qrss_MOC_SOURCES ${sdr_qrss_MOC_HEADERS})

set(sdr_qrss_HEADERS ${sdr_qrss_MOC_HEADERS} options.hh)

add_executable(sdr_qrss ${sdr_qrss_SOURCES} ${sdr_qrss_MOC_SOURCES})
target_link_libraries(sdr_qrss
 ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${LIBS} libsdr libsdr-gui)

