
HEADERS += \
    src/io/error.h \
    src/io/JsonInterface.h \
    src/io/Properties.h \
    src/core/Note.h \
    src/core/Bar.h \
    src/core/NoteStream.h \
    src/core/Score.h

SOURCES += \
    src/io/JsonInterface.cpp \
    src/io/Properties.cpp \
    src/io/Properties_json.cpp \
    src/io/Properties_qvariant_to_string.cpp \
    src/core/Note.cpp \
    src/core/Bar.cpp \
    src/core/NoteStream.cpp \
    src/core/Score.cpp
