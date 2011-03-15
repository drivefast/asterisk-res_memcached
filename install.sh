if [ -s include/asterisk.h ] ; then
	echo "please cd into the directory where the asterisk source has been untarred\n"
	exit
fi
cp asterisk-res_memcached/res_memcached.c res/.
cp asterisk-res_memcached/memcached.conf.sample configs/.
