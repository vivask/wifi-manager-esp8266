#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

# embed files from the "certs" directory as binary data symbols
# in the app
COMPONENT_SRCDIRS := src
COMPONENT_ADD_INCLUDEDIRS := include
COMPONENT_EMBED_FILES := dist/style.css dist/code.js dist/index.html dist/favicon.ico
# COMPONENT_EMBED_FILES := vue/style.css vue/code.js vue/index.html vue/favicon.ico