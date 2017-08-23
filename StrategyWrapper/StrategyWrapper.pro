TEMPLATE = subdirs

# subproject names
SUBDIRS += \
    DataManager #\
#    StreamingLibrary \
#    QSTECANMessage

# Where to find the sub projects (folder location)
DataManager.subdir = DataManager
#StreamingLibrary.subdir = StreamingLibrary
#QSTECANMessgae.subdir = QSTECANMessage

# dependencies of the sub projects
# DataManager.depends = QSTECANMessage
# DataManager.depends = StreamingLibrary

# StreamingLibrary.depends = QSTECANMessage

