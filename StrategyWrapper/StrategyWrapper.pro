TEMPLATE = subdirs

# subproject names
SUBDIRS += \
    DataManager \
    TCPProtocol \

# Where to find the sub projects (folder location)
DataManager.subdir = DataManager
TCPProtocol.subdir = TCPProtocol

# dependencies of the sub projects
DataManager.depends = TCPProtocol

