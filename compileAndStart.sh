make -s -j8 DEFAULT_TARGET=shared_release
export DYLD_LIBRARY_PATH=`pwd`/platform/lib/Darwin/x86_64
cd server
bin/Darwin/x86_64/macchina
