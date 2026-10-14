TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    cpp \
    qmlsetup \
    integration \
    startup

cpp.file = cpp/tst_patchbuilder.pro
qmlsetup.file = qmlsetup/qmlsetup.pro
integration.file = integration/tst_repository.pro
startup.file = startup/tst_startup.pro
