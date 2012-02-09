whatta...?
----------
__res_memcached__ is an asterisk wrapper module around libmemcached, a client library that offers 
access to memcached servers. memcached servers are distributed key-value stores, that work in RAM 
memory and can be accessed over the network; this makes them a good choice for a fast, intermediate 
layer between an application and the database backend. memcached can be found at http://memcached.org 
and libmemcached is available at http://libmemcached.org . you will need both of them as 
prerequisites.

basic functions
---------------
__res_memcached__ implements the basic memcached access functions: _get_, _set_, _add_, _replace_, 
_append_, _delete_. 

    exten => s,n,set(MCD(${key})=some text)
    exten => s,n,mcdappend(${key},... and some more text)
    exten => s,n,noop(value now: ${MCD(${key})}) ; prints "some text... and some more text" to CLI

in addition to this, __res_memcached__ offers a powerful counter function, that safely maintains 
integer counters across multiple concurrent clients and clusters of memcached servers.

install
-------
__res_memcached__ needs to be built into asterisk. i'm working with the people that take care of the 
asterisk distribution so we can include this module in the main distribution. until that happens, 
you will need to compile asterisk from source and have it take care of linking to libmemcached. 
therefore, step by step, this is what you have to do.

1. install memcached server (from http://code.google.com/p/memcached/downloads/list) on the servers 
where you want it working. install the client library libmemcached 
(from https://launchpad.net/libmemcached/+download) on the same system where you plan to install 
asterisk.

2. obtain the asterisk source code, from https://www.asterisk.org/downloads. unzip and untar it, but 
dont proceed to building it yet. 

3. cd into the directory where you unzipped / untarred asterisk, and get the __res_memcached__ module 
(git must be installed on your machine): `git clone git://github.com/drivefast/asterisk-res_memcached.git`

4. we now need to move the source files to their appropriate places in the asterisk directory. a 
shell script was provided for that, so run `./asterisk-res_memcached/install.sh`

5. edit the file `configure.ac` and add the following lines next to the similar ones:

> `AST_EXT_LIB_SETUP([MEMCACHED], [memcached client], [memcached])`
> `AST_EXT_LIB_CHECK([MEMCACHED], [memcached], [memcached_create], [libmemcached/memcached.h])`

6. edit the file `makeopts.in` and add the following lines next to the similar ones:

> `MEMCACHED_INCLUDE=@MEMCACHED_INCLUDE@`
> `MEMCACHED_LIB=@MEMCACHED_LIB@`

7. edit the file `build_tools/menuselect-deps.in` and add the following line next to the similar ones:

> `MEMCACHED=@PBX_MEMCACHED@`

8. run `./bootstrap.sh`. if you previously built from this asterisk directory, also do a `make clean`

9. only now proceed with building asterisk (`./configure; make menuconfig; make; make install`).

10. start your memcached servers. edit the file `/etc/asterisk/memcached.conf` and configure the 
startup parameters.

11. start asterisk, login to its console, and try `"core show function MCD"`. you should get an 
usage description.

what'd you get
--------------

a bunch of apps and functions:

- `__MCD(key)__` (r/w function) - gets or sets the value in the cache store for the given key
- `mcdadd(key,value)` (app) - same as above, but fail if the key exists
- `mcdreplace(key,value)` (app) - same as above, but fail if the key doesnt exist
- `mcdappend(key,value)` (app) - append given text to the value at an existing key
- `mcddelete(key)` (app) - delete an entry in the cache store
- `MCDCOUNTER(key)` (r/w function) - sets, increments, decrements or reads the value of an integer 
counter maintained in the cache store

none of the functions or the apps above would fail in such a way that it would terminate the call.  
if any of them would need to return an abnormal result, they would do so by setting the value of a 
dialplan variable called `MCDRESULT`. the values returned in `MCDRESULT` are the same as the ones 
documented for libmemcached. a few more values were added to the __res_memcached__ module:

* `MEMCACHED_ARGUMENT_NEEDED` - missing or invalid argument type in the app or function call
* `MEMCACHED_KEY_TOO_LONG` - key name is too long (maximum lenght is 64 characters)
* `MEMCACHED_VALUE_TOO_LONG` - value string is too long (maximum is 4096)
* `MEMCACHED_BAD_INCREMENT` - for MCDCOUNTER(), the increment needs to be an integer value
* `MEMCACHED_BINARY_PROTO_NEEDED` - for MCDCOUNTER(), the binary protocol has to be used

the connections to the servers are defined when the module is loaded, and they are based on the 
settings in the memcached.conf file (which ends up in the same directory where the other asterisk 
configuration files are). however, be advised that an actual tcp connection is only opened to the 
server once the first server operation is requested. for more information about operating with 
clusters of servers, see the memcached server documentation. res_memcached can connect to servers 
clusters. if so configured, the memcache module can force a global prefix to be added in front of 
each key it operates with. this is helpful for partitioning the data in the cache store (create some 
sort of tables).


apps and functions
------------------

- `__MCD(key)__` 

sets or returns the value for a key in the cache store. when written to, this function uses the 
'set' memcached operation.

> `key`: the key; may be prefixed with the value in the configuration file


- `__mcdset(key,value)__`

writes a value in the cache store with a given key. the key may exist, and its value is replaced 
with this new value; or may not exist, and it is created. the key is expired (deleted) automatically 
after a period of time (see the discussion about time-to-live below). the mcdset() dialplan app is 
an alternative to writing in the MCD() function.

> `key`: the key; may be prefixed with the value in the configuration file
>
> `value`: the value to be set for the given key


- `__mcdadd(key,value)__`

creates a key in the cache store and assigns the given value to it. if the key already exists, the 
operation fails and the error is returned in the MCDRESULT dialplan variable. the key is expired 
(deleted) automatically after a period of time (see the discussion about time-to-live below).

> `key`: the key; may be prefixed with the value in the configuration file
>
> `value`: the value to be set for the given key


- `__mcdreplace(key,value)__`

replaces the value for a key in the cache store. if the key doesnt exist, the operation fails and 
the error is returned in the MCDRESULT dialplan variable. the key is expired (deleted) automatically 
after a period of time (see the discussion about time-to-live below). 

> `key`: the key; may be prefixed with the value in the configuration file
>
> `value`: the value to be set for the given key


- `__mcdappend(key,text)__`

adds more text at the end of the current value of an existing key in the cache store. the operation 
is atomic: between the time when the app is called, until the time that it finishes its execution, 
the key is locked, and another memcached operation from another client would not be able to modify 
the value. the key is expired (deleted) automatically after a period of time (see the discussion 
about time-to-live below).

> `key`: the key; may be prefixed with the value in the configuration file
>
> `value`: the value to be set for the given key


- `__mcddelete(key)__`

deletes a key and its value from the cache store.

> `key`: the key; may be prefixed with the value in the configuration file


- `__MCDCOUNTER(key[,increment])__`

when written, the function creates or updates an integer entry in the cache store and forces it to 
the given numeric value. by default, the counter time-to-live is 0 (entry is persistent); if a 
limited time-to-live is needed, set the value of the MCDTTL dialplan variable to the desired value 
before creating the key.

when read, the number is initially incremented with the given value (or decremented if the value is 
negative), and the result is returned. the operation is atomic: between the time when the function 
is called, until the time that it finishes its execution, the key is locked, and another memcached 
operation from another client would not be able to modify the counter value.

> `key`: the key; may be prefixed with the value in the configuration file
>
> `increment` (only valid when reading): increment or decrement the value at the key, before 
      returning it

   
time-to-live
------------

in the memcached world, the keys are automatically expired after a period of time. this timeout 
value is expressed in seconds, and indicates the time after which the key-value entry is not 
guaranteed to exist in the cache store anymore. to make sure a key-value record is immediately 
deleted from the cache store, use the `mcddelete()` dialplan app. the default time-to-live interval 
is considered 0 by the server, which means the entry will never expire (be deleted from the cache). 
this doesnt mean, however, that the entry will survive a memcached server restart; memcached by 
itself has no mechanism to persist the data. 

the `ttl` parameter from the configuration file will set the standard time-to-live for the records 
in the cache store, and its implicit value is 0. in the dialplan you may override the time-to-live 
of a record that you are working with, by setting the value of the `MCDTTL` dialplan variable to the 
appropriate integer value. 


author, licensing and credits
-----------------------------

Radu Maierean
radu dot maierean at gmail

Copyright (c) 2010 Radu Maierean

the __res_memcached__ module is distributed under the GNU General Public License version 2. The GPL 
(version 2) is included in this source tree in the file COPYING.

the __res_memcached__ module is built on top of Brian Aker's libmemcached, and dynamically links to it. 
the __res_memcached module__ is intended to be used with asterisk, so you will have to follow their 
usage and distribution policy. and i guess so do i. i'm no lawyer and i have to take the safe route, 
and this is why i go with the same level of license restriction as asterisk does. 
