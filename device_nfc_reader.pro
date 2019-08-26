TEMPLATE = subdirs 
CONFIG += ordered 

SUBDIRS = \ 
        src/tools \
        src/com \

OTHER_FILES += \ 
            common.pri

# what subproject depends on others
app.depends = \ 
            tools/ \
            com/ \

SUBDIRS += app 
